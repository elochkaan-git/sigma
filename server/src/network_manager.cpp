#include "network_manager.h"

#include "command_types.h"
#include "logging.h"
#include "registry.h"
#include "server_responses.h"
#include "structures.h"
#include "wire_command_types.h"

#include <QAbstractSocket>
#include <QByteArray>
#include <QDateTime>
#include <QHostAddress>
#include <QIterable>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QSettings>
#include <QUuid>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QtLogging>
#include <QtTypes>

#include <chrono>
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

QString
formatBytes(qint64 bytes)
{
  const QStringList units = { "bytes", "KB", "MB", "GB", "TB" };
  double value = bytes;
  int unitIndex = 0;

  while (value >= 5 * 1024 && unitIndex < units.size() - 1) {
    value /= 1024.0;
    ++unitIndex;
  }

  return QString("%1 %2")
    .arg(QString::number(value, 'f', unitIndex == 0 ? 0 : 2))
    .arg(units[unitIndex]);
}

QJsonArray
serializeUsers(const std::optional<std::vector<User>>& users)
{
  QJsonArray arr;
  if (!users.has_value())
    return arr;
  for (const auto& u : users.value()) {
    QJsonObject o;
    o["user_id"] = static_cast<qint64>(u.user_id);
    o["login"] = u.login;
    arr.append(o);
  }
  return arr;
}

NetworkManager::NetworkManager(Dispatcher* dispatcher,
                               OnlineUsersRegistry* registry,
                               CallRegistry* call_registry,
                               const QString& iniPath)
  : mDispatcher(dispatcher)
  , mRegistry(registry)
  , mCallRegistry(call_registry)
  , mSettings(iniPath.isEmpty() ? "config.ini" : iniPath, QSettings::IniFormat)
{
  if (!mSettings.childGroups().contains("network")) {
    qWarning(appNetwork) << "There's no network section in config";
  }
  mSettings.beginGroup("network");
  // TODO: может, добавить проверку типа перед использованием?
  mConfig.host.setAddress(checkAndGetValue("host", "0.0.0.0").toString());
  mConfig.port = checkAndGetValue("port", 5555).toUInt();
  mConfig.flood_limit = checkAndGetValue("flood_limit", 5).toUInt();
  mConfig.ban_limit = checkAndGetValue("ban_limit", 60).toUInt();
  mConfig.trust_proxy_header =
    checkAndGetValue("trust_proxy_server", false).toBool();
  mConfig.max_msg_size =
    checkAndGetValue("max_msg_size", 536870912).toULongLong();
  mConfig.max_login_attempts =
    checkAndGetValue("max_login_attempts", 4).toInt();
  mConfig.max_oversized_msgs =
    checkAndGetValue("max_oversized_msgs", 4).toInt();
  mConfig.max_msgs_allowed = checkAndGetValue("max_msgs_allowed", 50).toInt();
  qInfo(appNetwork) << "Running server on" << mConfig.host.toString() << ":"
                    << mConfig.port;
  mSettings.endGroup();

  mServer = new QWebSocketServer(
    "p2p messenger", QWebSocketServer::NonSecureMode, this);
  QObject::connect(mServer,
                   &QWebSocketServer::newConnection,
                   this,
                   &NetworkManager::onNewConnection);
  mServer->listen(mConfig.host, mConfig.port);
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
      [](const GetFriendsResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["friends"] = serializeUsers(r.friends);
        return wrap("get_friends_response", std::move(payload));
      },
      [](const GetFriendRequestsResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["requests"] = serializeUsers(r.requests);
        return wrap("get_friend_requests_response", std::move(payload));
      },
      [](const GetSentFriendRequestsResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["sent_requests"] = serializeUsers(r.sent_requests);
        return wrap("get_sent_friend_requests_response", std::move(payload));
      },
      [](const GetServerStatsResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["online"] = static_cast<qint64>(r.online);
        payload["total"] = static_cast<qint64>(r.total);
        return wrap("get_server_stats_response", std::move(payload));
      },
      [](const StartCallResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        return wrap("start_call_response", std::move(payload));
      },
      [](const IncomingCallResponse& r) {
        QJsonObject payload;
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        payload["caller_id"] = static_cast<qint64>(r.caller_id);
        payload["with_video"] = r.with_video;
        return wrap("incoming_call", std::move(payload));
      },
      [](const AcceptCallResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        return wrap("accept_call_response", std::move(payload));
      },
      [](const CallAcceptedResponse& r) {
        QJsonObject payload;
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        return wrap("call_accepted", std::move(payload));
      },
      [](const RejectCallResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        return wrap("reject_call_response", std::move(payload));
      },
      [](const CallRejectedResponse& r) {
        QJsonObject payload;
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        return wrap("call_rejected", std::move(payload));
      },
      [](const EndCallResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        return wrap("end_call_response", std::move(payload));
      },
      [](const CallEndedResponse& r) {
        QJsonObject payload;
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        return wrap("call_ended", std::move(payload));
      },
      [](const SdpResponse& r) {
        QJsonObject payload;
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        payload["sdp"] = r.sdp;
        return wrap("sdp", std::move(payload));
      },
      [](const IceCandidateResponse& r) {
        QJsonObject payload;
        payload["call_id"] = r.call_id.toString(QUuid::WithoutBraces);
        payload["candidate"] = r.candidate;
        payload["mid"] = r.mid;
        return wrap("ice_candidate", std::move(payload));
      },
      [](const Error& r) {
        QJsonObject payload;
        payload["reason"] = r.reason;
        return wrap("error", std::move(payload));
      },
      [](const GetTurnCredentialsResponse& r) {
        QJsonObject payload;
        payload["status"] = QJsonValue(static_cast<int>(r.status));
        payload["username"] = r.username;
        payload["password"] = r.password;
        payload["ttl"] = r.ttl;
        return wrap("get_turn_credentials_response", std::move(payload));
      },
    },
    response);

  qInfo(appNetwork) << "Preparing" << obj["type"] << "command";
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

Command
NetworkManager::deserialize(const QUuid& client_id, const QByteArray& message)
{
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);
  Error error_response;
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    error_response.client_id = client_id;
    error_response.reason = "Can't parse JSON document";
    return error_response;
  }

  QJsonObject obj = doc.object();
  std::optional<QString> error =
    validate(obj, { { "type", isString }, { "payload", [](const QJsonValue& v) {
                                             return v.isObject();
                                           } } });
  if (error.has_value()) {
    error_response.client_id = client_id;
    error_response.reason = error.value();
    return error_response;
  }

  QString type = obj["type"].toString();
  QJsonObject payload = obj["payload"].toObject();
  qInfo(appNetwork) << "New message from" << client_id << ", type:" << type;

  auto it = kCommandSpecs.find(type);
  if (it == kCommandSpecs.end()) {
    error_response.client_id = client_id;
    error_response.reason = "No such type of command";
    return error_response;
  }
  const CommandSpec& spec = *it;

  unsigned int user_id = 0;
  if (spec.requiresAuth) {
    if (mConnections.contains(client_id)) {
      QVariant uid = this->mConnections.value(client_id)->property("user_id");
      if (!uid.isValid()) {
        error_response.client_id = client_id;
        error_response.reason = "You are not logged in";
        return error_response;
      }
      user_id = uid.toUInt();
    } else {
      error_response.client_id = client_id;
      error_response.reason = "No such socket";
      return error_response;
    }
  }

  error = validate(payload, spec.fields);
  if (error.has_value()) {
    error_response.client_id = client_id;
    error_response.reason = error.value();
    return error_response;
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
                   &QWebSocket::binaryMessageReceived,
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

  mIDConstraints.removeIf(
    [&](ConnectionState& r) { return r.status == ConnectionStatus::COMMON; });
  mIPConstraints.removeIf(
    [&](ConnectionState& r) { return r.status == ConnectionStatus::COMMON; });
}

void
NetworkManager::onMessageReceived(const QByteArray& message)
{
  qDebug(appNetwork) << "Size of message:" << message.size();
  QWebSocket* client = qobject_cast<QWebSocket*>(this->sender());
  QHostAddress addr(
    client->request().headers().value("X-Real-IP").toByteArray());
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
      if (!mConfig.trust_proxy_header) {
        addr = client->peerAddress();
      }
      mIPConstraints[addr] =
        ConnectionState{ ConnectionStatus::COMMON, {}, now };
    }
    user = mIPConstraints.find(addr);
  }

  ConnectionState& state =
    std::visit([](auto&& it) -> auto& { return *it; }, user);
  CommandType type;
  Error error_cmd;

  if (message.size() > mConfig.max_msg_size) {
    const QString reason("Message size bigger than " +
                         formatBytes(message.size()));
    error_cmd.client_id = client_id;
    error_cmd.reason = reason;
    cmd = error_cmd;
    type = CommandType::OVERSIZED;
  } else {
    cmd = this->deserialize(client_id, message);
    type = getTypeOfCommand(cmd);
  }

  using namespace std::chrono;
  if (state.status != ConnectionStatus::COMMON) {
    long timeLimitMinutes = 0;
    QString reason = "";
    milliseconds delta = now - state.timestamp;
    switch (state.status) {
      case ConnectionStatus::FLOOD:
        timeLimitMinutes = mConfig.flood_limit;
        reason = "FLOOD";
        break;
      case ConnectionStatus::BAN:
        timeLimitMinutes = mConfig.ban_limit;
        reason = "BAN";
        break;
      default:
        timeLimitMinutes = mConfig.flood_limit;
        reason = "UNKNOWN";
        break;
    }
    if (duration_cast<minutes>(delta).count() < timeLimitMinutes) {
      error_cmd.client_id = client_id;
      error_cmd.reason =
        QString("You cannot send messages due to a %1").arg(reason);
      cmd = error_cmd;
    } else {
      state.status = ConnectionStatus::COMMON;
      state.last_cmds.clear();
      state.last_cmds.append({ type, now });
    }
  } else {
    state.last_cmds.removeIf([&](const Record& r) {
      if (r.type == CommandType::LOGIN || r.type == CommandType::OVERSIZED) {
        return duration_cast<minutes>(now - r.timestamp).count() >
               mConfig.ban_limit;
      } else {
        return duration_cast<minutes>(now - r.timestamp).count() >
               mConfig.flood_limit;
      }
    });
    QHash<CommandType, qsizetype> cmdTypesCount;
    for (const auto& cmd : state.last_cmds) {
      if (cmdTypesCount.contains(cmd.type)) {
        cmdTypesCount[cmd.type] += 1;
      } else {
        cmdTypesCount[cmd.type] = 1;
      }
    }
    if (cmdTypesCount.value(CommandType::LOGIN, 0) >
        mConfig.max_login_attempts) {
      error_cmd.client_id = client_id;
      error_cmd.reason =
        QString("Too many login attempts. Now you in ban for %1 minutes")
          .arg(mConfig.ban_limit);
      cmd = error_cmd;
      state.last_cmds.clear();
      state.status = ConnectionStatus::BAN;
      state.timestamp = now;
    } else if (cmdTypesCount.value(CommandType::OVERSIZED, 0) >
               mConfig.max_oversized_msgs) {
      error_cmd.client_id = client_id;
      error_cmd.reason =
        QString("Too many big messages. Now you in ban for %1 minutes")
          .arg(mConfig.ban_limit);
      cmd = error_cmd;
      state.last_cmds.clear();
      state.status = ConnectionStatus::BAN;
      state.timestamp = now;
    } else if (state.last_cmds.size() > mConfig.max_msgs_allowed) {
      state.status = ConnectionStatus::FLOOD;
      state.timestamp = now;
      error_cmd.client_id = client_id;
      error_cmd.reason =
        QString("Too many messages. Now you block for %1 minutes due to flood")
          .arg(mConfig.flood_limit);
      cmd = error_cmd;
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
  QWebSocket* sender = qobject_cast<QWebSocket*>(this->sender());
  QUuid client_id = sender->property("client_id").toUuid();
  unsigned int user_id = sender->property("user_id").toUInt();
  mConnections.remove(client_id);
  mRegistry->removeUser(user_id);
  mCallRegistry->deleteRecord(user_id);
  qInfo(appNetwork) << client_id << "disconnected";
  sender->deleteLater();

  mIDConstraints.removeIf(
    [&](ConnectionState& r) { return r.status == ConnectionStatus::COMMON; });
  mIPConstraints.removeIf(
    [&](ConnectionState& r) { return r.status == ConnectionStatus::COMMON; });
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

QVariant
NetworkManager::checkAndGetValue(const QString& key,
                                 const QVariant& defaultValue)
{
  if (!mSettings.contains(key)) {
    qWarning(appNetwork)
      << QString("'%1' key not provided in config, using default value")
           .arg(key);
  }
  return mSettings.value(key, defaultValue);
}
