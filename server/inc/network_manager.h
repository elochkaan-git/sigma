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

class NetworkManager : public QObject
{
  Q_OBJECT

public:
  NetworkManager(Dispatcher* dispatcher, OnlineUsersRegistry* registry);
  QByteArray serialize(const Response& response);
  Command deserialize(QUuid client_id, const QByteArray& message);
  void sendResponse(const Response& response);

public slots:
  void onNewConnection();
  void onMessageReceived(const QString& message);
  void onDisconnected();
  void onErrorOccured(QAbstractSocket::SocketError error);

private:
  Dispatcher* mDispatcher;
  OnlineUsersRegistry* mRegistry;
  QWebSocketServer* mServer;
  QHash<QUuid, QWebSocket*> mConnections;
  QUuid getClientId(const Response& response);
  void handleSideEffect(const Response& response);
};