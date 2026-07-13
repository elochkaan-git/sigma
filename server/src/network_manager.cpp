#include "network_manager.h"

#include "command_types.h"
#include "commands.h"
#include "logging.h"
#include "registry.h"
#include "responses.h"
#include "structures.h"

#include <QAbstractSocket>
#include <QByteArray>
#include <QDateTime>
#include <QHostAddress>
#include <QIterable>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QUuid>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QtLogging>
#include <QtTypes>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <optional>
#include <variant>

QJsonObject
wrap(const char* type, QJsonObject payload)
{
  QJsonObject o;
  o["type"] = type;
  o["payload"] = std::move(payload);
  return o;
}

std::optional<QString>
validate(const QJsonObject& obj, const std::vector<FieldSpec>& fields)
{
  for (const auto& f : fields) {
    if (!obj.contains(f.name)) {
      return QString("Missing field: %1").arg(f.name);
    }
    if (!f.check(obj[f.name])) {
      return QString("Invalid type for field: %1").arg(f.name);
    }
  }
  return std::nullopt;
}

NetworkManager::NetworkManager(Dispatcher* dispatcher,
                               OnlineUsersRegistry* registry)
  : mDispatcher(dispatcher)
  , mRegistry(registry)
{
  mServer = new QWebSocketServer(
    "p2p messenger", QWebSocketServer::NonSecureMode, this);
  QObject::connect(mServer,
                   &QWebSocketServer::newConnection,
                   this,
                   &NetworkManager::onNewConnection);
  mServer->listen(QHostAddress::Any, 5555);
  qInfo(appNetwork) << "Server start listening";
}

QByteArray
NetworkManager::serialize(const Response& response)
{
  QJsonObject obj = std::visit(
    overloaded{
      [](const RegisterUserResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        return wrap("register_user_response", std::move(payload));
      },
      [](const LoginUserResponse& r) {
        QJsonObject payload;
        payload["user_id"] = static_cast<qint64>(r.user_id);
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        return wrap("login_user_response", std::move(payload));
      },
      [](const SendMessageResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        return wrap("send_message_response", std::move(payload));
      },
      [](const NewMessageResponse& r) {
        QJsonObject payload;
        payload["sender_id"] = static_cast<qint64>(r.sender_id);
        payload["content"] = r.content;
        return wrap("new_message", std::move(payload));
      },
      [](const SendFriendRequestResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        return wrap("send_friend_request_response", std::move(payload));
      },
      [](const AcceptFriendRequestResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        return wrap("accept_friend_request_response", std::move(payload));
      },
      [](const RejectFriendRequestResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        return wrap("reject_friend_request_response", std::move(payload));
      },
      [](const RemoveFriendResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        return wrap("remove_friend_response", std::move(payload));
      },
      [](const Error& r) {
        QJsonObject payload;
        payload["reason"] = r.reason;
        return wrap("error", std::move(payload));
      } },
    response);

  qInfo(appNetwork) << "Preparing" << obj["type"] << "command";
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

Command
NetworkManager::deserialize(QUuid client_id, const QByteArray& message)
{
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    return Error{ client_id, "Can't parse JSON document" };
  }

  QJsonObject obj = doc.object();
  std::optional<QString> error =
    validate(obj, { { "type", isString }, { "payload", [](const QJsonValue& v) {
                                             return v.isObject();
                                           } } });
  if (error.has_value()) {
    return Error{ client_id, error.value() };
  }

  QString type = obj["type"].toString();
  QJsonObject payload = obj["payload"].toObject();
  qInfo(appNetwork) << "New message from" << client_id << ", type:" << type;

  auto it = kCommandSpecs.find(type);
  if (it == kCommandSpecs.end()) {
    return Error{ client_id, "No such type of command" };
  }
  const CommandSpec& spec = *it;

  unsigned int user_id = 0;
  if (spec.requiresAuth) {
    QVariant uid = this->mConnections[client_id]->property("user_id");
    if (!uid.isValid()) {
      return Error{ client_id, "You are not logged in" };
    }
    user_id = uid.toUInt();
  }

  error = validate(payload, spec.fields);
  if (error.has_value()) {
    return Error{ client_id, error.value() };
  }

  return spec.build(client_id, user_id, payload);
}

void
NetworkManager::sendResponse(const Response& response)
{
  QUuid id = this->getClientId(response);
  if (!mConnections.contains(id))
    return;
  QByteArray message = serialize(response);
  qInfo(appNetwork) << "Sending response to" << id.toString();
  mConnections[id]->sendBinaryMessage(message);
}

void
NetworkManager::onNewConnection()
{
  QWebSocket* newConnection = mServer->nextPendingConnection();
  QObject::connect(newConnection,
                   &QWebSocket::textMessageReceived,
                   this,
                   &NetworkManager::onMessageReceived);
  QObject::connect(newConnection,
                   &QWebSocket::disconnected,
                   this,
                   &NetworkManager::onDisconnected);
  QObject::connect(newConnection,
                   &QWebSocket::errorOccurred,
                   this,
                   &NetworkManager::onErrorOccured);

  QUuid newConnectionId = QUuid::createUuid();
  newConnection->setProperty("client_id", newConnectionId);
  mConnections[newConnectionId] = newConnection;
  qInfo(appNetwork) << "New connection! ID: " << newConnectionId;
}

// Перевести на чтение бинарных данных, а не текстовых
void
NetworkManager::onMessageReceived(const QString& message)
{
  qDebug(appNetwork) << "Size of message:" << message.size();
  qsizetype maxMsgSize = 536'870'912;
  QWebSocket* client = qobject_cast<QWebSocket*>(this->sender());
  QHostAddress addr(client->request().headers().value("IPv4").toByteArray());
  QUuid client_id = client->property("client_id").toUuid();

  QDateTime now = QDateTime::currentDateTime();
  Command cmd;
  QVariant uid = client->property("user_id");
  std::variant<decltype(mIDConstraints)::iterator,
               decltype(mIPConstraints)::iterator>
    user;

  if (uid.isValid()) {
    user = mIDConstraints.find(uid.toUInt());
    auto IDuser = std::get<decltype(mIDConstraints)::iterator>(user);
    if (IDuser == mIDConstraints.end()) {
      mIDConstraints[uid.toUInt()] =
        ConnectionState{ ConnectionStatus::COMMON, {}, now };
    }
    user = mIDConstraints.find(uid.toUInt());
  } else {
    user = mIPConstraints.find(addr);
    auto IPuser = std::get<decltype(mIPConstraints)::iterator>(user);
    if (IPuser == mIPConstraints.end()) {
      // TODO: с появлением кофигурации сделать обращение к peerAddress, если
      // передача через заголовок отключена
      mIPConstraints[addr] =
        ConnectionState{ ConnectionStatus::COMMON, {}, now };
    }
    user = mIPConstraints.find(addr);
  }

  ConnectionState& state =
    std::visit([](auto&& it) -> auto& { return *it; }, user);
  CommandType type;

  if (message.size() > maxMsgSize) {
    // TODO: сделать небольшую функцию, которая бы в зависимости от размера
    // писала mb, kb и т.д.
    QString reason("Message size bigger than %1 MB");
    cmd = Error{ client_id, reason.arg(maxMsgSize / 1024 / 1024) };
    type = CommandType::OVERSIZED;
  } else {
    const QByteArray data(message.toStdString());
    cmd = this->deserialize(client_id, data);
    type = this->getTypeOfCommand(cmd);
  }

  using namespace std::chrono;
  if (state.status != ConnectionStatus::COMMON) {
    long timeLimitMinutes = 0;
    QString reason = "";
    milliseconds delta = now - state.timestamp;
    switch (state.status) {
      case ConnectionStatus::FLOOD:
        timeLimitMinutes = 15;
        reason = "FLOOD";
        break;
      case ConnectionStatus::BAN:
        timeLimitMinutes = 60;
        reason = "BAN";
        break;
      default:
        timeLimitMinutes = 15;
        reason = "UNKNOWN";
        break;
    }
    if (duration_cast<minutes>(delta).count() < timeLimitMinutes) {
      cmd =
        Error{ client_id,
               QString("You cannot send messages due to a %1").arg(reason) };
    } else {
      state.status = ConnectionStatus::COMMON;
      state.last_cmds.clear();
      state.last_cmds.append({ type, now });
    }
  } else {
    state.last_cmds.removeIf([&](const Record& r) {
      return duration_cast<minutes>(now - r.timestamp).count() > 15;
    });
    // TODO: подсчет всех типов, вместо if для каждого типа
    if (std::count_if(
          state.last_cmds.begin(), state.last_cmds.end(), [](const Record& r) {
            return r.type == CommandType::LOGIN;
          }) > 4) {
      cmd =
        Error{ client_id, QString("Too many login attempts. Now you in ban") };
      state.last_cmds.clear();
      state.status = ConnectionStatus::BAN;
      state.timestamp = now;
    } else if (std::count_if(state.last_cmds.begin(),
                             state.last_cmds.end(),
                             [](const Record& r) {
                               return r.type == CommandType::OVERSIZED;
                             }) > 4) {
      cmd =
        Error{ client_id, QString("Too many big messages. Now you in ban") };
      state.last_cmds.clear();
      state.status = ConnectionStatus::BAN;
      state.timestamp = now;
    } else if (state.last_cmds.size() > 50) {
      state.status = ConnectionStatus::FLOOD;
      state.timestamp = now;
      cmd = Error{
        client_id,
        QString(
          "Too many messages. Now you block for 15 minutes due to flood")
      };
    } else {
      state.last_cmds.append({ type, now });
    }
  }

  mDispatcher->dispatch(cmd, this, [this](const Response& r) {
    this->handleSideEffect(r);
    this->sendResponse(r);
  });
}

void
NetworkManager::onDisconnected()
{
  // TODO: logging
  QWebSocket* sender = qobject_cast<QWebSocket*>(this->sender());
  mConnections.remove(sender->property("client_id").toUuid());
  mRegistry->removeUser(sender->property("user_id").toUInt());
  qInfo(appNetwork) << sender->property("client_id").toUuid() << "disconnected";
  sender->deleteLater();
}

void
NetworkManager::onErrorOccured(QAbstractSocket::SocketError error)
{
  qWarning(appNetwork) << error;
}

QUuid
NetworkManager::getClientId(const Response& response)
{
  return std::visit(overloaded{ [](const auto& r) { return r.client_id; } },
                    response);
}

CommandType
NetworkManager::getTypeOfCommand(const Command& cmd)
{
  return std::visit(
    overloaded{
      [](const Error&) { return CommandType::ERROR; },
      [](const RegisterUser&) { return CommandType::REGISTER; },
      [](const LoginUser&) { return CommandType::LOGIN; },
      [](const SendMessage&) { return CommandType::SEND_MESSAGE; },
      [](const SendFriendRequest&) { return CommandType::SEND_FRIEND_REQUEST; },
      [](const AcceptFriendRequest&) {
        return CommandType::ACCEPT_FRIEND_REQUEST;
      },
      [](const RejectFriendRequest&) {
        return CommandType::REJECT_FRIEND_REQUEST;
      },
      [](const RemoveFriend&) { return CommandType::REMOVE_FRIEND; } },
    cmd);
}

void
NetworkManager::handleSideEffect(const Response& response)
{
  std::visit(overloaded{ [this](const LoginUserResponse& r) {
                          if (!mConnections.contains(r.client_id))
                            return;
                          this->mRegistry->registerUser(r.user_id, r.client_id);
                          this->mConnections[r.client_id]->setProperty(
                            "user_id", r.user_id);
                        },
                         [](const auto&) {} },
             response);
}