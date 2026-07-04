#pragma once
#include "command_types.h"
#include "dispatcher.h"
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QHash>
#include <QUuid>
#include <QByteArray>
#include <QAbstractSocket>

class NetworkManager : public QObject
{
  Q_OBJECT

public:
  NetworkManager(Dispatcher* dispatcher);
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
  QWebSocketServer* mServer;
  QHash<QUuid, QWebSocket*> mConnections;
  QUuid getClientId(const Response& response);
};