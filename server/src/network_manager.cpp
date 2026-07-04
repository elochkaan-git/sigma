#include "network_manager.h"
#include "command_types.h"
#include "commands.h"
#include "registry.h"
#include "responses.h"
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
#include <qtypes.h>
#include <variant>

NetworkManager::NetworkManager(Dispatcher* dispatcher,
                               OnlineUsersRegistry* registry)
  : mDispatcher(dispatcher)
  , mRegistry(registry)
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
  // TODO: обработка всех вариантов ответа (пока что временная заглушка)
  QJsonObject obj = std::visit(
    overloaded{ [](const RegisterUserResponse& r) {
                 QJsonObject o;
                 o["client_id"] = r.client_id.toString(QUuid::WithoutBraces);
                 o["status"] = QJsonValue(static_cast<int>(r.status));
                 return o;
               },
                [](const LoginUserResponse& r) {
                  QJsonObject o;
                  o["client_id"] = r.client_id.toString(QUuid::WithoutBraces);
                  o["user_id"] = static_cast<qint64>(r.user_id);
                  o["status"] = QJsonValue(static_cast<int>(r.status));
                  return o;
                },
                [](const SendMessageResponse& r) {
                  QJsonObject o;
                  o["client_id"] = r.client_id.toString(QUuid::WithoutBraces);
                  o["status"] = QJsonValue(static_cast<int>(r.status));
                  return o;
                },
                [](const NewMessageResponse& r) {
                  QJsonObject o;
                  o["client_id"] = r.client_id.toString(QUuid::WithoutBraces);
                  o["sender_id"] = static_cast<qint64>(r.sender_id);
                  o["content"] = r.content;
                  return o;
                },
                [](const auto&) { return QJsonObject{}; } },
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
  if (obj["type"] == "register") {
    return RegisterUser{ client_id,
                         obj["login"].toString(),
                         obj["pwd"].toString() };
  } else if (obj["type"] == "login") {
    return LoginUser{ client_id,
                      obj["login"].toString(),
                      obj["pwd"].toString() };
  } else if (obj["type"] == "message") {
    return SendMessage{ client_id,
                        static_cast<unsigned int>(obj["sender_id"].toInteger()),
                        static_cast<unsigned int>(
                          obj["receiver_id"].toInteger()),
                        obj["content"].toString() };
  } else {
    return NullCommand{};
  }
}

void
NetworkManager::sendResponse(const Response& response)
{
  QUuid id = this->getClientId(response);
  if (!mConnections.contains(id))
    return;
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
  mDispatcher->dispatch(cmd, this, [this](const Response& r) {
    this->handleSideEffect(r);
    this->sendResponse(r);
  });
}

void
NetworkManager::onDisconnected()
{
  // TODO: logging
  QWebSocket* sender = qobject_cast<QWebSocket*>(this->sender());
  mConnections.remove(sender->property("client_id").toUuid());
  mRegistry->removeUser(sender->property("user_id").toUInt());
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