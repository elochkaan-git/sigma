#pragma once
#include "command_types.h"
#include "dispatcher.h"
#include "registry.h"
#include "server_commands.h"

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
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QtTypes>

#include <functional>
#include <qstringview.h>

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

/**
 * @brief Спецификация команды
 *
 */
struct CommandSpec
{
  std::vector<FieldSpec>
    fields;          /**< спецификации полей, которые нужно проверить */
  bool requiresAuth; /**< требует ли команда авторизации */
  std::function<
    Command(QUuid client_id, unsigned int user_id, const QJsonObject& payload)>
    build; /**< функция, собирающая и возвращающая команду */
};

/**
 * @brief Статус соединения пользователя
 *
 */
enum class ConnectionStatus
{
  COMMON = 0, /**< Обычное состояние */
  FLOOD = 1,  /**< Наказан за флуд */
  BAN = 2 /**< Наказан за серьезные проступки (брутфорс, перегрузка сервера) */
};

/**
 * @brief Запись о присланном запросе
 *
 */
struct Record
{
  CommandType type;    /**< тип команды */
  QDateTime timestamp; /**< время прихода */
};

/**
 * @brief Состояние подключения пользователя
 *
 */
struct ConnectionState
{
  ConnectionStatus status; /**< статус подключения */
  QList<Record> last_cmds; /**< список последних команд */
  QDateTime timestamp;     /**< временная метка, когда был изменен статус */
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
   * @see server_responses.h
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
   * @see server_commands.h
   */
  Command deserialize(const QUuid& client_id, const QByteArray& message);
  /**
   * @brief Отправляет сообщение сокету с ID, указанному в Response. Если
   * пользователь не в сети, то ничего не отправляется
   *
   * @param response ответ от Dispatcher
   * @see server_responses.h
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
  void onMessageReceived(const QByteArray& message);
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
  /**
   * @brief Структура, хранящая конфигурацию класс
   * на момент запуска. Заполняется в конструкторе
   */
  struct NetworkConfig
  {
    QHostAddress host;
    qint64 max_msg_size;
    quint32 port;
    quint32 flood_limit;
    quint32 ban_limit;
    quint8 max_login_attempts;
    quint8 max_oversized_msgs;
    quint8 max_msgs_allowed;
    bool trust_proxy_header;
  } mConfig;

private:
  /**
   * @brief Возвращает QUuid пользователя из ответа
   *
   * @param response ответ от Dispatcher
   * @return QUuid ID сокета
   * @see server_responses.h
   */
  QUuid getClientId(const Response& response);
  /**
   * @brief Обрабатывает побочные эффекты ответов.
   *
   * Например, для RegisterUserResponse достаточно отправить ответ пользователю,
   * а при LoginUserResponse нужно добавить пользователя в OnlineUsersRegistry
   *
   * @param response ответ от Dispatcher
   * @see server_responses.h
   */
  void handleSideEffect(const Response& response);
  /**
   * @brief Проверяет, существует ли переданный ключ в настройках,
   * загруженных из ini-файла. Если ключ отсутствует, то возвращается
   * значение по умолчанию
   *
   * @param key ключ
   * @param defaultValue значение по умолчанию
   * @return QVariant значение, если ключ существует
   */
  QVariant checkAndGetValue(const QString& key, const QVariant& defaultValue);
};

/**
 * @brief Проверяет, что значение является строкой
 */
static const auto isString = [](const QJsonValue& v) { return v.isString(); };
/**
 * @brief Проверяет, что значение является числом
 */
static const auto isNumber = [](const QJsonValue& v) { return v.isDouble(); };

/**
 * @brief Словарь спецификаций комманд
 */
inline const QHash<QString, CommandSpec> kCommandSpecs = {
  { "register_user",
    { { { "login", isString }, { "pwd", isString } },
      false,
      [](QUuid client_id, unsigned int, const QJsonObject& p) -> Command {
        RegisterUser cmd;
        cmd.client_id = client_id;
        cmd.login = p["login"].toString();
        cmd.pwd = p["pwd"].toString();
        return cmd;
      } } },

  { "login_user",
    { { { "login", isString }, { "pwd", isString } },
      false,
      [](QUuid client_id, unsigned int, const QJsonObject& p) -> Command {
        LoginUser cmd;
        cmd.client_id = client_id;
        cmd.login = p["login"].toString();
        cmd.pwd = p["pwd"].toString();
        return cmd;
      } } },

  { "send_friend_request",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        SendFriendRequest cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        cmd.friend_id = static_cast<unsigned int>(p["friend_id"].toInteger());
        return cmd;
      } } },

  { "accept_friend_request",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        AcceptFriendRequest cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        cmd.friend_id = static_cast<unsigned int>(p["friend_id"].toInteger());
        return cmd;
      } } },

  { "reject_friend_request",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        RejectFriendRequest cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        cmd.friend_id = static_cast<unsigned int>(p["friend_id"].toInteger());
        return cmd;
      } } },

  { "remove_friend",
    { { { "friend_id", isNumber } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        RemoveFriend cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        cmd.friend_id = static_cast<unsigned int>(p["friend_id"].toInteger());
        return cmd;
      } } },

  { "send_message",
    { { { "receiver_id", isNumber }, { "content", isString } },
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject& p)
        -> Command {
        SendMessage cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        cmd.receiver_id =
          static_cast<unsigned int>(p["receiver_id"].toInteger());
        cmd.content = p["content"].toString();
        return cmd;
      } } },

  { "get_friends",
    { {},
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject&) -> Command {
        GetFriends cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        return cmd;
      } } },

  { "get_friend_requests",
    { {},
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject&) -> Command {
        GetFriendRequests cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        return cmd;
      } } },

  { "get_sent_friend_requests",
    { {},
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject&) -> Command {
        GetSentFriendRequests cmd;
        cmd.client_id = client_id;
        cmd.user_id = user_id;
        return cmd;
      } } },

  { "get_server_stats",
    { {},
      false,
      [](QUuid client_id, unsigned int, const QJsonObject&) -> Command {
        GetServerStats cmd;
        cmd.client_id = client_id;
        return cmd;
      } } }
};