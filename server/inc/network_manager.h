#pragma once
#include "command_types.h"
#include "commands.h"
#include "dispatcher.h"
#include "registry.h"

#include <QAbstractSocket>
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QtTypes>

#include <functional>

struct FieldSpec
{
  QString name;
  std::function<bool(const QJsonValue&)> check;
};

struct CommandSpec
{
  std::vector<FieldSpec> fields;
  bool requiresAuth;
  std::function<
    Command(QUuid client_id, unsigned int user_id, const QJsonObject& payload)>
    build;
};

enum class ConnectionStatus
{
  COMMON = 0,
  FLOOD = 1,
  BAN = 2
};

enum class CommandType
{
  ERROR,
  REGISTER,
  LOGIN,
  SEND_MESSAGE,
  SEND_FRIEND_REQUEST,
  ACCEPT_FRIEND_REQUEST,
  REJECT_FRIEND_REQUEST,
  REMOVE_FRIEND,
  OVERSIZED
};

struct Record
{
  CommandType type;
  QDateTime timestamp;
};

struct ConnectionState
{
  ConnectionStatus status;
  QList<Record> last_cmds;
  QDateTime timestamp;
};

/**
 * @brief Сетевой класс.
 *
 * Сетевой класс для обработки входящих подключений, а также принятием и
 * отправкой сообщений. Не изменяет содержимого ответов в методе sendResponse
 *
 */
class NetworkManager : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Конструктор класса NetworkManager
   *
   * @param dispatcher указатель на объект класса Dispatcher
   * @param registry указатель на объект класса OnlineUsersRegistry
   */
  NetworkManager(Dispatcher* dispatcher,
                 OnlineUsersRegistry* registry,
                 const QString& iniPath);
  /**
   * @brief Сериализует Response в Json
   *
   * @param response ответ Response
   * @return QByteArray Json в виде массива байтов
   * @see responses.h
   */
  QByteArray serialize(const Response& response);
  /**
   * @brief Преобразует полученное сообщение в команду, передаваемую в
   * Dispatcher::dispatch
   *
   * @param client_id ID сокета, от которого пришло сообщение
   * @param message сообщение
   * @return Command команда
   * @see Dispatcher
   * @see commands.h
   */
  Command deserialize(const QUuid& client_id, const QByteArray& message);
  /**
   * @brief Отправляет сообщение сокету с ID, указанному в Response. Если
   * пользователь не в сети, то ничего не отправляется
   *
   * @param response ответ от Dispatcher
   * @see responses.h
   * @see Dispatcher
   * @see OnlineUsersRegistry
   */
  void sendResponse(const Response& response);

public slots:
  /**
   * @brief Вызывается при новом подключении
   */
  void onNewConnection();
  /**
   * @brief Вызывается при получении сообщения
   */
  void onMessageReceived(const QString& message);
  /**
   * @brief Вызывается при отключении сокета
   */
  void onDisconnected();
  /**
   * @brief Вызывается при возникновении сетевой ошибки
   */
  void onErrorOccured(QAbstractSocket::SocketError error);

private:
  Dispatcher* mDispatcher;
  OnlineUsersRegistry* mRegistry;
  QWebSocketServer* mServer;
  QHash<QUuid, QWebSocket*> mConnections;
  QHash<unsigned int, ConnectionState> mIDConstraints;
  QHash<QHostAddress, ConnectionState> mIPConstraints;
  QSettings mSettings;
  struct NetworkConfig
  {
    QHostAddress host;
    quint64 max_msg_size;
    quint32 port;
    quint32 flood_limit;
    quint32 ban_limit;
    bool trust_proxy_header;
  } mConfig;

private:
  /**
   * @brief Возвращает QUuid пользователя из ответа
   *
   * @param response ответ от Dispatcher
   * @return QUuid ID сокета
   * @see responses.h
   */
  QUuid getClientId(const Response& response);
  CommandType getTypeOfCommand(const Command& cmd);
  /**
   * @brief Обрабатывает побочные эффекты ответов.
   *
   * Например, для RegisterUserResponse достаточно отправить ответ пользователю,
   * а при LoginUserResponse нужно добавить пользователя в OnlineUsersRegistry
   *
   * @param response ответ от Dispatcher
   * @see responses.h
   */
  void handleSideEffect(const Response& response);
  QVariant checkAndGetValue(const QString& key, const QVariant& defaultValue);
};

static const auto isString = [](const QJsonValue& v) { return v.isString(); };
static const auto isNumber = [](const QJsonValue& v) { return v.isDouble(); };

inline const QHash<QString, CommandSpec> kCommandSpecs = {
  { "register_user",
    { { { "login", isString }, { "pwd", isString } },
      false,
      [](QUuid client_id, unsigned int, const QJsonObject& p) -> Command {
        return RegisterUser{ client_id,
                             p["login"].toString(),
                             p["pwd"].toString() };
      } } },
  { "login_user",
    { { { "login", isString }, { "pwd", isString } },
      false,
      [](QUuid client_id, unsigned int, const QJsonObject& p) -> Command {
        return LoginUser{ client_id,
                          p["login"].toString(),
                          p["pwd"].toString() };
      } } },
  { "send_friend_request",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        return SendFriendRequest{ client_id,
                                  user_id,
                                  static_cast<unsigned int>(
                                    p["friend_id"].toInteger()) };
      } } },
  { "accept_friend_request",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        return AcceptFriendRequest{ client_id,
                                    user_id,
                                    static_cast<unsigned int>(
                                      p["friend_id"].toInteger()) };
      } } },
  { "reject_friend_request",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        return RejectFriendRequest{ client_id,
                                    user_id,
                                    static_cast<unsigned int>(
                                      p["friend_id"].toInteger()) };
      } } },
  { "remove_friend",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        return RemoveFriend{ client_id,
                             user_id,
                             static_cast<unsigned int>(
                               p["friend_id"].toInteger()) };
      } } },
  { "send_message",
    { { { "receiver_id", isNumber }, { "content", isString } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        return SendMessage{
          client_id,
          user_id,
          static_cast<unsigned int>(
            p["receiver_id"].toInteger()), // было obj[...] — баг
          p["content"].toString()
        };
      } } },
};