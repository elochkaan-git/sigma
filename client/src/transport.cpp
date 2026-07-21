#include "transport.h"

#include "command_types.h"
#include "commands.h"
#include "logging.h"
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
#include <QSettings>
#include <QUuid>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketProtocol>
#include <QtLogging>
#include <QtTypes>

#include <optional>

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

Transport::Transport(QObject*)
{
  mSocket = new QWebSocket("", QWebSocketProtocol::VersionLatest, this);
  QObject::connect(mSocket, &QWebSocket::connected, this, &Transport::onConnected);
  QObject::connect(mSocket, &QWebSocket::disconnected, this, &Transport::onDisconnected);
  QObject::connect(mSocket, &QWebSocket::errorOccurred, this, &Transport::onErrorOccured);
  QObject::connect(mSocket, &QWebSocket::binaryMessageReceived, this, &Transport::onMessageReceived);
  QObject::connect(mSocket, &QWebSocket::sslErrors, this, &Transport::onSslErrors);
}

Transport::~Transport()
{
  this->disconnectFromHost();
}

std::optional<QByteArray>
Transport::serialize(const Command& cmd)
{
  std::optional<QJsonObject> obj = std::visit(
    overloaded{
      [](const wire::RegisterUser& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["login"] = cmd.login;
        payload["pwd"] = cmd.pwd;
        return wrap("register_user", std::move(payload));
      },
      [](const wire::LoginUser& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["login"] = cmd.login;
        payload["pwd"] = cmd.pwd;
        return wrap("login_user", std::move(payload));
      },
      [](const wire::SendMessage& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["receiver_id"] = static_cast<qint64>(cmd.receiver_id);
        payload["content"] = cmd.content;
        return wrap("send_message", std::move(payload));
      },
      [](const wire::SendFriendRequest& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["friend_id"] = static_cast<qint64>(cmd.friend_id);
        return wrap("send_friend_request", std::move(payload));
      },
      [](const wire::AcceptFriendRequest& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["friend_id"] = static_cast<qint64>(cmd.friend_id);
        return wrap("accept_friend_request", std::move(payload));
      },
      [](const wire::RejectFriendRequest& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["friend_id"] = static_cast<qint64>(cmd.friend_id);
        return wrap("reject_friend_request", std::move(payload));
      },
      [](const wire::RemoveFriend& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["friend_id"] = static_cast<qint64>(cmd.friend_id);
        return wrap("remove_friend", std::move(payload));
      },
      [](const wire::GetFriends&) -> std::optional<QJsonObject> {
        QJsonObject payload;
        return wrap("get_friends", std::move(payload));
      },
      [](const wire::GetFriendRequests&) -> std::optional<QJsonObject> {
        QJsonObject payload;
        return wrap("get_friend_requests", std::move(payload));
      },
      [](const wire::GetSentFriendRequests&) -> std::optional<QJsonObject> {
        QJsonObject payload;
        return wrap("get_sent_friend_requests", std::move(payload));
      },
      [](const wire::GetServerStats&) -> std::optional<QJsonObject> {
        QJsonObject payload;
        return wrap("get_server_stats", std::move(payload));
      },
      [](const wire::StartCall& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["callee_id"] = static_cast<qint64>(cmd.callee_id);
        payload["with_video"] = cmd.with_video;
        return wrap("start_call", std::move(payload));
      },
      [](const wire::AcceptCall& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["call_id"] = cmd.call_id.toString(QUuid::WithoutBraces);
        return wrap("accept_call", std::move(payload));
      },
      [](const wire::RejectCall& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["call_id"] = cmd.call_id.toString(QUuid::WithoutBraces);
        return wrap("reject_call", std::move(payload));
      },
      [](const wire::EndCall& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["call_id"] = cmd.call_id.toString(QUuid::WithoutBraces);
        return wrap("end_call", std::move(payload));
      },
      [](const wire::Sdp& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["call_id"] = cmd.call_id.toString(QUuid::WithoutBraces);
        payload["sdp"] = cmd.sdp;
        return wrap("sdp", std::move(payload));
      },
      [](const wire::IceCandidate& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["call_id"] = cmd.call_id.toString(QUuid::WithoutBraces);
        payload["candidate"] = cmd.candidate;
        payload["mid"] = cmd.mid;
        return wrap("ice_candidate", std::move(payload));
      },
      [](const wire::GetTurnCredentials&) -> std::optional<QJsonObject> {
        QJsonObject payload;
        return wrap("get_turn_credentials", std::move(payload));
      },
      [](const wire::SetAvatar& cmd) -> std::optional<QJsonObject> {
        QJsonObject payload;
        payload["avatar"] = cmd.avatar;
        return wrap("set_avatar", std::move(payload));
      },
      [](const wire::GetOnlineUsers&) -> std::optional<QJsonObject> {
        QJsonObject payload;
        return wrap("get_online_users", std::move(payload));
      },
      [](const auto& cmd) -> std::optional<QJsonObject> {
        // Ветка для всех остальных команд, которые не
        // должны обрабатываться. Например, клиент не должен посылать
        // команду Error
        qCritical(appNetwork) << QString("Not allowed command: %1").arg(commandTypeToString(getTypeOfCommand(cmd)));
        return std::nullopt;
      } },
    cmd);
  
  if (obj.has_value()) {
    qInfo(appNetwork) << "Preparing" << obj.value()["type"] << "command";
    return QJsonDocument(obj.value()).toJson(QJsonDocument::Compact);
  } else {
    return std::nullopt;
  }
}

Response
Transport::deserialize(const QByteArray& message)
{
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);
  wire::Error error_response;
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    error_response.reason = "Can't parse JSON document";
    return error_response;
  }

  QJsonObject obj = doc.object();
  std::optional<QString> error =
    validate(obj, { { "type", isString }, { "payload", [](const QJsonValue& v) {
                                             return v.isObject();
                                           } } });
  if (error.has_value()) {
    error_response.reason = error.value();
    return error_response;
  }

  QString type = obj["type"].toString();
  QJsonObject payload = obj["payload"].toObject();
  qInfo(appNetwork) << QString("New message from server! Type: %1").arg(type);

  auto it = kResponseSpecs.find(type);
  if (it == kResponseSpecs.end()) {
    error_response.reason = "No such type of response";
    return error_response;
  }
  const ResponseSpec& spec = *it;

  error = validate(payload, spec.fields);
  if (error.has_value()) {
    error_response.reason = error.value();
    return error_response;
  }
  return spec.build(payload);
}

void
Transport::sendCommand(const Command& cmd)
{
  std::optional<QByteArray> message = serialize(cmd);
  wire::Error error_response;
  if (!message.has_value()) {
    error_response.reason = "Not allowed command";
    emit this->responseReady(error_response);
    return;
  }
  qInfo(appNetwork) << "Sending command to server";
  if (mSocket->state() != QAbstractSocket::ConnectedState) {
    qWarning(appNetwork) << "Can't send command, because socket not connected yet";
    error_response.reason = "Socket not connected";
    emit this->responseReady(error_response);
  } else {
    mSocket->sendBinaryMessage(message.value());
  }
}

void
Transport::connectToHost(const QString& url)
{
  if (mSocket->state() == QAbstractSocket::ConnectedState) {
    qWarning(appNetwork) << "Socket already connected";
  } else if (mSocket->state() == QAbstractSocket::UnconnectedState) {
    mSocket->open(url);
  } else {
    qInfo(appNetwork) << "Socket in process of opening or closing";
  }
}

void
Transport::disconnectFromHost()
{
  if (mSocket->state() == QAbstractSocket::ConnectedState) {
    qInfo(appNetwork) << "Disconnecting";
    mSocket->close(QWebSocketProtocol::CloseCodeNormal);
  } else {
    qInfo(appNetwork) << "Socket not connected";
  }
}

void
Transport::onConnected()
{
  qInfo(appNetwork) << "Connected to server!";
  this->isConnected = true;
  emit connected();
}

void
Transport::onMessageReceived(const QByteArray& message)
{
  qDebug(appNetwork) << "Size of message:" << message.size();
  Response r = this->deserialize(message);
  emit this->responseReady(r);
}

void
Transport::onDisconnected()
{
  qInfo(appNetwork) << QString("Socket was closed. Reason: %1").arg(mSocket->closeReason());
  isConnected = false;
  if (mSocket->closeCode() != QWebSocketProtocol::CloseCodeNormal) {
    wire::Error error_response{ mSocket->closeReason() };
    emit this->responseReady(error_response);
  }
}

void
Transport::onErrorOccured(QAbstractSocket::SocketError)
{
  qWarning(appNetwork) << mSocket->errorString();
  if (!isConnected) {
    wire::Error error_response{ mSocket->errorString() };
    emit this->responseReady(error_response);
  }
}

void
Transport::onSslErrors(const QList<QSslError>& errors)
{
  qCritical(appNetwork) << "Errors have occurred while establishing the identity of the peer";
  for (const auto& e : errors) {
    qCritical(appNetwork) << e.errorString();
  }
  mSocket->abort();
}