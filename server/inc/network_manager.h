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
   * @see responses.h
   */
  QUuid getClientId(const Response& response);
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
  { "get_friends",
    { {},
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject&) -> Command {
        return GetFriends{ client_id, user_id };
      } } },
  { "get_friend_requests",
    { {},
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject&) -> Command {
        return GetFriendRequests{ client_id, user_id };
      } } },
  { "get_sent_friend_requests",
    { {},
      true,
      [](QUuid client_id, unsigned int user_id, const QJsonObject&) -> Command {
        return GetSentFriendRequests{ client_id, user_id };
      } } }
};