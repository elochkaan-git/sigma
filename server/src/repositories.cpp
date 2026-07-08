// TODO: пересмотреть ошибки
#include "repositories.h"
#include "connection_manager.h"
#include "logging.h"
#include "structures.h"
#include <QtLogging>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <optional>
#include <qlogging.h>
#include <vector>

UserRepository::UserRepository(ConnectionManager* manager)
  : mConnManager(manager)
{
}

OperationStatus
UserRepository::registerUser(QString login, QString pwd_hash)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status =
    query.prepare("insert into users (login, passwd) values (:login, :pwd)");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    return OperationStatus::InternalError;
  }
  query.bindValue(":login", login);
  query.bindValue(":pwd", pwd_hash);
  status = query.exec();
  if (status) {
    return OperationStatus::OK;
  } else if (query.lastError().nativeErrorCode() == "23505") {
    return OperationStatus::UserExist;
  } else {
    qWarning(appDatabase) << query.lastError().text();
    return OperationStatus::InternalError;
  }
}

std::pair<OperationStatus, std::optional<UserCredentials>>
UserRepository::findUserByLogin(QString login)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status =
    query.prepare("select id, login, passwd from users where login = :login");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    return { OperationStatus::InternalError, std::nullopt };
  }
  query.bindValue(":login", login);
  status = query.exec();
  bool isValue = query.next();
  if (status && isValue) {
    unsigned int user_id = query.value(0).toUInt();
    QString user_login = query.value(1).toString();
    QString user_pwd_hash = query.value(2).toString();
    return { OperationStatus::OK,
             UserCredentials{ user_id, user_login, user_pwd_hash } };
  } else if (status && !isValue) {
    qWarning(appDatabase) << "No user with such login found";
    return { OperationStatus::UserNotExist, std::nullopt };
  } else {
    qCritical(appDatabase) << query.lastError().text();
    return { OperationStatus::InternalError, std::nullopt };
  }
}

MessageRepository::MessageRepository(ConnectionManager* manager)
  : mConnManager(manager)
{
}

OperationStatus
MessageRepository::saveToQueue(unsigned int sender_id,
                               unsigned int receiver_id,
                               QString content)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("insert into msgs_queue (sender_id, receiver_id, "
                              "content, sent_at) values (:sender, :receiver, "
                              ":content, current_timestamp)");
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":sender", sender_id);
  query.bindValue(":receiver", receiver_id);
  query.bindValue(":content", content);
  status = query.exec();
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    // TODO: Replace throwing error on returning warning
  }
}

std::pair<OperationStatus, std::optional<std::vector<Message>>>
MessageRepository::getQueuedMessages(unsigned int receiver_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status =
    query.prepare("select * from msgs_queue where receiver_id = :user");
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":user", receiver_id);
  status = query.exec();
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    // TODO: Replace throwing error on returning warning
  }
  std::vector<Message> msgs;
  msgs.reserve(100);
  while (query.next()) {
    unsigned int msg_id = query.value(0).toUInt();
    unsigned int sender_id = query.value(1).toUInt();
    QString content = query.value(3).toString();
    msgs.push_back({ msg_id, sender_id, content });
  }
  if (!msgs.size()) {
    return { OperationStatus::OK, std::nullopt };
  } else {
    return { OperationStatus::OK, msgs };
  }
}

OperationStatus
MessageRepository::deleteFromQueue(unsigned int msg_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("delete from msgs_queue where id = :id");
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":id", msg_id);
  status = query.exec();
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    // TODO: Replace throwing error on returning warning
  }
}