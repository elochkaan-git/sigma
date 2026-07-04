#include "network_manager.h"
#include "command_types.h"
#include "commands.h"
#include <QAbstractSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QUuid>
#include <QWebSocket>
#include <QWebSocketServer>
#include <qlogging.h>

NetworkManager::NetworkManager(Dispatcher* dispatcher)
  : mDispatcher(dispatcher)
{
  mServer = new QWebSocketServer(
    "p2p messenger", QWebSocketServer::NonSecureMode, this);
  QObject::connect(mServer,
                   &QWebSocketServer::newConnection,
                   this,
                   &NetworkManager::onNewConnection);
  mServer->listen(QHostAddress::Any, 5555);
}

QByteArray
NetworkManager::serialize(const Response& response)
{
  QJsonObject obj =
    std::visit(overloaded{ [](const RegisterUserResponse& r) {
                 QJsonObject o;
                 o["client_id"] = r.client_id.toString(QUuid::WithoutBraces);
                 o["status"] = QJsonValue(static_cast<int>(r.status));
                 return o;
               } },
               response);

  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

Command
NetworkManager::deserialize(QUuid client_id, const QByteArray& message)
{
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);

  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    // TODO: proper error handling/logging for malformed input
    return RegisterUser{};
  }

  QJsonObject obj = doc.object();
  return RegisterUser{ client_id,
                       obj["login"].toString(),
                       obj["pwd"].toString() };
}

void
NetworkManager::sendResponse(const Response& response)
{
  QUuid id = this->getClientId(response);
  QByteArray message = serialize(response);
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
}

void
NetworkManager::onMessageReceived(const QString& message)
{
  const QByteArray data(message.toStdString());
  QUuid client_id =
    qobject_cast<QWebSocket*>(this->sender())->property("client_id").toUuid();
  Command cmd = this->deserialize(client_id, data);
  mDispatcher->dispatch(
    cmd, this, [this](const Response& r) { this->sendResponse(r); });
}

void
NetworkManager::onDisconnected()
{
  // TODO: logging
  QWebSocket* sender = qobject_cast<QWebSocket*>(this->sender());
  mConnections.remove(sender->property("client_id").toUuid());
  sender->deleteLater();
}

void
NetworkManager::onErrorOccured(QAbstractSocket::SocketError error)
{
  // TODO: logging
  return;
}

QUuid
NetworkManager::getClientId(const Response& response)
{
  return std::visit(
    overloaded{ [](const RegisterUserResponse& r) { return r.client_id; } },
    response);
}