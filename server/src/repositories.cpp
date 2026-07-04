#include "repositories.h"
#include "connection_manager.h"
#include "structures.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <optional>
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
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":login", login);
  query.bindValue(":pwd", pwd_hash);
  status = query.exec();
  if (status) {
    return OperationStatus::OK;
  } else if (query.lastError().nativeErrorCode() == "23505") {
    return OperationStatus::UserExist;
  } else {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
}

std::optional<UserCredentials>
UserRepository::findUserByLogin(QString login)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("select * from users where login = :login");
  if (!status) {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":login", login);
  status = query.exec();
  bool temp = query.next();
  if (status && temp) {
    unsigned int user_id = query.value(0).toUInt();
    QString user_login = query.value(1).toString();
    QString user_pwd_hash = query.value(2).toString();
    return UserCredentials{ user_id, user_login, user_pwd_hash };
  } else {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  } //FIXME: Возможна ошибка из-за отсутствия пользователя в системе
}

MessageRepository::MessageRepository(ConnectionManager* manager)
  : mConnManager(manager)
{
}

void
MessageRepository::saveToQueue(unsigned int sender_id,
                               unsigned int receiver_id,
                               QString content)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("insert into msgs_query (sender_id, receiver_id, "
                              "content, sent_at) values (:sender, :receiver, :content, current_timestamp)");
  if (!status) {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":sender", sender_id);
  query.bindValue(":receiver", receiver_id);
  query.bindValue(":content", content);
  status = query.exec();
  if (!status) {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
}

std::vector<Message>
MessageRepository::getQueuedMessages(unsigned int user_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status =
    query.prepare("select * from msgs_query where receiver_id = :user");
  if (!status) {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":user", user_id);
  status = query.exec();
  if (!status) {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
  std::vector<Message> msgs;
  msgs.reserve(100);
  while (query.next()) {
    unsigned int msg_id = query.value(0).toUInt();
    unsigned int sender_id = query.value(1).toUInt();
    QString content = query.value(3).toString();
    msgs.push_back({ msg_id, sender_id, content });
  }
  return msgs;
}

void
MessageRepository::deleteFromQueue(unsigned int msg_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("delete from msgs_query where id = :id");
  if (!status) {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
  query.bindValue(":id", msg_id);
  status = query.exec();
  if (!status) {
    throw std::runtime_error(
      query.lastError()
        .text()
        .toStdString()); // TODO: Replace throwing error on returning warning
  }
}