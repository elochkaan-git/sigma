#pragma once
#include "command_types.h"
#include "commands.h"
#include "responses.h"
#include "structures.h"

#include <QAbstractSocket>
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QSslError>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QtTypes>

#include <functional>
#include <optional>

/**
 * @brief Спецификация поля в запросе
 */
struct FieldSpec
{
  QString name; /**< название поля */
  std::function<bool(const QJsonValue&)>
    check; /**<  функция, которая должна проверить значение поля (тип и/или
              значение) */
};

struct ResponseSpec
{
  std::vector<FieldSpec>
    fields;          /**< спецификации полей, которые нужно проверить */
  std::function<Response(const QJsonObject& payload)>
    build; /**< функция, собирающая и возвращающая команду */
};

class Transport : public QObject
{
  Q_OBJECT

signals:
  void responseReady(const Response& response);
  void connected();

public:
  explicit Transport(QObject* parent = nullptr);
  std::optional<QByteArray> serialize(const Command& cmd);
  Response deserialize(const QByteArray& message);
  void sendCommand(const Command& cmd);
  void connectToHost(const QString& url);
  void disconnectFromHost();
  ~Transport();

public slots:
  void onConnected();
  void onMessageReceived(const QByteArray& message);
  void onDisconnected();
  void onErrorOccured(QAbstractSocket::SocketError);
  void onSslErrors(const QList<QSslError>& errors);

private:
  QWebSocket* mSocket;
  bool isConnected = false;
};

/**
 * @brief Проверяет, что значение является строкой
 */
static const auto isString = [](const QJsonValue& v) { return v.isString(); };
/**
 * @brief Проверяет, что значение является числом
 */
static const auto isNumber = [](const QJsonValue& v) { return v.isDouble(); };

static const auto isBool = [](const QJsonValue& v) { return v.isBool(); };

/**
 * @brief Проверяет, что значение является строкой, представляющей корректный
 * QUuid
 */
static const auto isUuid = [](const QJsonValue& v) {
  if (!v.isString())
    return false;
  return !QUuid::fromString(v.toString()).isNull();
};

/**
 * @brief Преобразует QJsonValue (строку) в QUuid
 */
static inline QUuid
parseUuid(const QJsonValue& v)
{
  return QUuid::fromString(v.toString());
}

/**
 * @brief Проверяет, что значение является объектом User
 * ({ "user_id": number, "login": string, "avatar": string,
 * "last_seen": string })
 */
static const auto isUserObject = [](const QJsonValue& v) {
  if (!v.isObject())
    return false;
  const QJsonObject obj = v.toObject();
  return obj.contains("user_id") && obj["user_id"].isDouble() &&
         obj.contains("login") && obj["login"].isString() &&
         obj.contains("avatar") && obj["avatar"].isString() &&
         obj.contains("last_seen") && obj["last_seen"].isString();
};
 
/**
 * @brief Проверяет, что значение является массивом объектов User
 */
static const auto isUserArray = [](const QJsonValue& v) {
  if (!v.isArray())
    return false;
  const QJsonArray arr = v.toArray();
  for (const QJsonValue& item : arr) {
    if (!isUserObject(item))
      return false;
  }
  return true;
};
 
/**
 * @brief Преобразует QJsonValue (объект User) в структуру User
 */
static inline User
parseUser(const QJsonValue& v)
{
  const QJsonObject obj = v.toObject();
  QDateTime last_seen;
  if (obj["last_seen"].isString()) {
    last_seen =
      QDateTime::fromString(obj["last_seen"].toString(), Qt::ISODate);
  }
  return User{ static_cast<unsigned int>(obj["user_id"].toInt()),
               obj["login"].toString(),
               obj["avatar"].toString(),
               last_seen };
}
 
/**
 * @brief Преобразует QJsonValue (массив User) в std::vector<User>
 */
static inline std::vector<User>
parseUserArray(const QJsonValue& v)
{
  std::vector<User> result;
  const QJsonArray arr = v.toArray();
  result.reserve(arr.size());
  for (const QJsonValue& item : arr) {
    result.push_back(parseUser(item));
  }
  return result;
}
 
/**
 * @brief Словарь спецификаций ответов
 */
inline const QHash<QString, ResponseSpec> kResponseSpecs = {
  { "register_user_response",
    { { { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::RegisterUserResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "login_user_response",
    { { { "user_id", isNumber }, { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::LoginUserResponse r;
        r.user_id = static_cast<unsigned int>(p["user_id"].toInt());
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "send_message_response",
    { { { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::SendMessageResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "send_friend_request_response",
    { { { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::SendFriendRequestResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "accept_friend_request_response",
    { { { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::AcceptFriendRequestResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "reject_friend_request_response",
    { { { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::RejectFriendRequestResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "new_message",
    { { { "sender_id", isNumber }, { "content", isString } },
      [](const QJsonObject& p) -> Response {
        wire::NewMessageResponse r;
        r.sender_id = static_cast<unsigned int>(p["sender_id"].toInt());
        r.content = p["content"].toString();
        return r;
      } } },

  { "remove_friend_response",
    { { { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::RemoveFriendResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "get_friends_response",
    { { { "status", isNumber }, { "friends", isUserArray } },
      [](const QJsonObject& p) -> Response {
        wire::GetFriendsResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.friends = parseUserArray(p["friends"]);
        return r;
      } } },

  { "get_friend_requests_response",
    { { { "status", isNumber }, { "requests", isUserArray } },
      [](const QJsonObject& p) -> Response {
        wire::GetFriendRequestsResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.requests = parseUserArray(p["requests"]);
        return r;
      } } },

  { "get_sent_friend_requests_response",
    { { { "status", isNumber }, { "sent_requests", isUserArray } },
      [](const QJsonObject& p) -> Response {
        wire::GetSentFriendRequestsResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.sent_requests = parseUserArray(p["sent_requests"]);
        return r;
      } } },

  { "get_server_stats_response",
    { { { "status", isNumber },
        { "online", isNumber },
        { "total", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::GetServerStatsResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.online = static_cast<unsigned int>(p["online"].toInt());
        r.total = static_cast<unsigned int>(p["total"].toInt());
        return r;
      } } },

  { "error",
    { { { "reason", isString } },
      [](const QJsonObject& p) -> Response {
        wire::Error r;
        r.reason = p["reason"].toString();
        return r;
      } } },

  { "start_call_response",
    { { { "status", isNumber }, { "call_id", isUuid } },
      [](const QJsonObject& p) -> Response {
        wire::StartCallResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.call_id = parseUuid(p["call_id"]);
        return r;
      } } },

  { "incoming_call",
    { { { "call_id", isUuid }, { "caller_id", isNumber }, { "with_video", isBool } },
      [](const QJsonObject& p) -> Response {
        wire::IncomingCallResponse r;
        r.call_id = parseUuid(p["call_id"]);
        r.caller_id = static_cast<unsigned int>(p["caller_id"].toInt());
        r.with_video = p["with_video"].toBool();
        return r;
      } } },

  { "accept_call_response",
    { { { "status", isNumber }, { "call_id", isUuid } },
      [](const QJsonObject& p) -> Response {
        wire::AcceptCallResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.call_id = parseUuid(p["call_id"]);
        return r;
      } } },

  { "call_accepted",
    { { { "call_id", isUuid } },
      [](const QJsonObject& p) -> Response {
        wire::CallAcceptedResponse r;
        r.call_id = parseUuid(p["call_id"]);
        return r;
      } } },

  { "reject_call_response",
    { { { "status", isNumber }, { "call_id", isUuid } },
      [](const QJsonObject& p) -> Response {
        wire::RejectCallResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.call_id = parseUuid(p["call_id"]);
        return r;
      } } },

  { "call_rejected",
    { { { "call_id", isUuid } },
      [](const QJsonObject& p) -> Response {
        wire::CallRejectedResponse r;
        r.call_id = parseUuid(p["call_id"]);
        return r;
      } } },

  { "end_call_response",
    { { { "status", isNumber }, { "call_id", isUuid } },
      [](const QJsonObject& p) -> Response {
        wire::EndCallResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.call_id = parseUuid(p["call_id"]);
        return r;
      } } },

  { "call_ended",
    { { { "call_id", isUuid } },
      [](const QJsonObject& p) -> Response {
        wire::CallEndedResponse r;
        r.call_id = parseUuid(p["call_id"]);
        return r;
      } } },

  { "sdp",
    { { { "call_id", isUuid }, { "sdp", isString }, { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::SdpResponse r;
        r.call_id = parseUuid(p["call_id"]);
        r.sdp = p["sdp"].toString();
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "ice_candidate",
    { { { "call_id", isUuid },
        { "candidate", isString },
        { "mid", isString },
        { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::IceCandidateResponse r;
        r.call_id = parseUuid(p["call_id"]);
        r.candidate = p["candidate"].toString();
        r.mid = p["mid"].toString();
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },
  { "get_turn_credentials_response",
    { { { "status", isNumber },
        { "username", isString },
        { "password", isString },
        { "ttl", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::GetTurnCredentialsResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.username = p["username"].toString();
        r.password = p["password"].toString();
        r.ttl = p["ttl"].toInt();
        return r;
      } } },

  { "set_avatar_response",
    { { { "status", isNumber } },
      [](const QJsonObject& p) -> Response {
        wire::SetAvatarResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        return r;
      } } },

  { "get_online_users_response",
    { { { "status", isNumber }, { "users", isUserArray } },
      [](const QJsonObject& p) -> Response {
        wire::GetOnlineUsersResponse r;
        r.status = OperationStatus{ p["status"].toInt() };
        r.users = parseUserArray(p["users"]);
        return r;
      } } }
};