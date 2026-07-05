#pragma once
#include "command_types.h"
#include "dispatcher.h"
#include "registry.h"
#include <QAbstractSocket>
#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QUuid>
#include <QWebSocket>
#include <QWebSocketServer>

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
  NetworkManager(Dispatcher* dispatcher, OnlineUsersRegistry* registry);
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
  Command deserialize(QUuid client_id, const QByteArray& message);
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
};