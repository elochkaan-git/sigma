#include "repositories.h"

#include "connection_manager.h"
#include "logging.h"
#include "structures.h"

#include <QtLogging>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <cstddef>
#include <optional>
#include <qhashfunctions.h>
#include <qlogging.h>
#include <vector>

UserRepository::UserRepository(ConnectionManager* manager)
  : mConnManager(manager)
{
}

OperationStatus
UserRepository::registerUser(const QString& login, const QString& pwd_hash)
{
  // FIXME: добавить откат транзакции, чтобы при UserExists не было перехода на следующий id
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

std::pair<OperationStatus, std::optional<User>>
UserRepository::getUserByLogin(const QString& login)
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
    return { OperationStatus::OK, User{ user_id, user_login, user_pwd_hash } };
  } else if (status && !isValue) {
    qWarning(appDatabase) << "No user with login '" << login << "' found";
    return { OperationStatus::UserNotExist, std::nullopt };
  } else {
    qCritical(appDatabase) << query.lastError().text();
    return { OperationStatus::InternalError, std::nullopt };
  }
}

std::pair<OperationStatus, std::optional<User>>
UserRepository::getUserByID(unsigned int user_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status =
    query.prepare("select id, login, passwd from users where id = :id");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    return { OperationStatus::InternalError, std::nullopt };
  }
  query.bindValue(":id", user_id);
  status = query.exec();
  bool isValue = query.next();
  if (status && isValue) {
    unsigned int user_id = query.value(0).toUInt();
    QString user_login = query.value(1).toString();
    QString user_pwd_hash = query.value(2).toString();
    return { OperationStatus::OK, User{ user_id, user_login, user_pwd_hash } };
  } else if (status && !isValue) {
    qWarning(appDatabase) << "No user with id '" << user_id << "' found";
    return { OperationStatus::UserNotExist, std::nullopt };
  } else {
    qCritical(appDatabase) << query.lastError().text();
    return { OperationStatus::InternalError, std::nullopt };
  }
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
UserRepository::getUsersById(const std::vector<unsigned int>& ids)
{
  if (ids.empty()) {
    qWarning(appDatabase) << "Empty vector of ids";
    return { OperationStatus::UserNotExist, std::nullopt };
  }
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  QString placeholders = "(";
  for (size_t i = 0; i < ids.size() - 1; ++i) {
    placeholders += QString(":id%1,").arg(i);
  }
  placeholders += QString(":id%1)").arg(ids.size() - 1);

  bool status = query.prepare(
    "select id, login, passwd from users where id in " + placeholders);
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    return { OperationStatus::InternalError, std::nullopt };
  }
  for (size_t i = 0; i < ids.size(); ++i) {
    query.bindValue(QString(":id%1").arg(i), ids.at(i));
  }
  status = query.exec();
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    return { OperationStatus::InternalError, std::nullopt };
  }
  std::vector<User> users;
  users.reserve(ids.size());
  while (query.next()) {
    unsigned int user_id = query.value(0).toUInt();
    QString user_login = query.value(1).toString();
    QString user_pwd_hash = query.value(2).toString();
    users.push_back(User{ user_id, user_login, user_pwd_hash });
  }
  if (users.empty()) {
    qWarning(appDatabase) << "No such users, sorry";
    return { OperationStatus::OK, std::nullopt };
  }
  return { OperationStatus::OK, users };
}

MessageRepository::MessageRepository(ConnectionManager* manager)
  : mConnManager(manager)
{
}

OperationStatus
MessageRepository::saveToQueue(unsigned int sender_id,
                               unsigned int receiver_id,
                               const QString& content)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("insert into msgs_queue (sender_id, receiver_id, "
                              "content, sent_at) values (:sender, :receiver, "
                              ":content, current_timestamp)");
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    return OperationStatus::InternalError;
  }
  query.bindValue(":sender", sender_id);
  query.bindValue(":receiver", receiver_id);
  query.bindValue(":content", content);
  status = query.exec();
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    return OperationStatus::InternalError;
  }
  return OperationStatus::OK;
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
    return { OperationStatus::InternalError, std::nullopt };
  }
  query.bindValue(":user", receiver_id);
  status = query.exec();
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    return { OperationStatus::InternalError, std::nullopt };
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
    return OperationStatus::InternalError;
  }
  query.bindValue(":id", msg_id);
  status = query.exec();
  if (!status) {
    qWarning(appDatabase) << query.lastError().text().toStdString();
    return OperationStatus::InternalError;
  }
  return OperationStatus::OK;
}

RelationRepository::RelationRepository(ConnectionManager* manager)
  : mConnManager(manager)
{
}

OperationStatus
RelationRepository::sendFriendRequest(unsigned int user_id,
                                      unsigned int friend_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  bool status = connection.transaction();
  if (!status) {
    qWarning(appDatabase) << "Can't begin transaction. What:"
                          << connection.lastError().text();
    return OperationStatus::InternalError;
  }
  QSqlQuery query(connection);
  status = query.prepare("insert into relations (user_id, friend_id, status) "
                         "values (:user_id, :friend_id, 'sent')");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  OperationStatus error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  status = query.prepare("insert into relations (user_id, friend_id, status) "
                         "values (:friend_id, :user_id, 'received')");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  connection.commit();
  return OperationStatus::OK;
}

OperationStatus
RelationRepository::acceptFriendRequest(unsigned int user_id,
                                        unsigned int friend_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  bool status = connection.transaction();
  if (!status) {
    qWarning(appDatabase) << "Can't begin transaction. What:"
                          << connection.lastError().text();
    return OperationStatus::InternalError;
  }
  QSqlQuery query(connection);
  status = query.prepare("update relations set status = 'friend' where user_id "
                         "= :user_id and friend_id = :friend_id");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  OperationStatus error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  status = query.prepare("update relations set status = 'friend' where user_id "
                         "= :friend_id and friend_id = :user_id");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  connection.commit();
  return OperationStatus::OK;
}

OperationStatus
RelationRepository::rejectFriendRequest(unsigned int user_id,
                                        unsigned int friend_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  bool status = connection.transaction();
  if (!status) {
    qWarning(appDatabase) << "Can't begin transaction. What:"
                          << connection.lastError().text();
    return OperationStatus::InternalError;
  }
  QSqlQuery query(connection);
  status = query.prepare("delete from relations where user_id = :user_id and "
                         "friend_id = :friend_id and status = 'received'");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  OperationStatus error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  status = query.prepare("delete from relations where user_id = :friend_id and "
                         "friend_id = :user_id and status = 'sent'");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  connection.commit();
  return OperationStatus::OK;
}

OperationStatus
RelationRepository::removeFriend(unsigned int user_id, unsigned int friend_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  bool status = connection.transaction();
  if (!status) {
    qWarning(appDatabase) << "Can't begin transaction. What:"
                          << connection.lastError().text();
    return OperationStatus::InternalError;
  }
  QSqlQuery query(connection);
  status = query.prepare("delete from relations where user_id = :user_id and "
                         "friend_id = :friend_id and status = 'friend'");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  OperationStatus error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  status = query.prepare("delete from relations where user_id = :friend_id and "
                         "friend_id = :user_id and status = 'friend'");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text();
    connection.rollback();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    connection.rollback();
    return error;
  }

  connection.commit();
  return OperationStatus::OK;
}

std::pair<OperationStatus, std::optional<std::vector<unsigned int>>>
RelationRepository::getFriendsID(unsigned int user_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("select friend_id from relations where user_id = "
                              ":user_id and status = 'friend'");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    return { OperationStatus::InternalError, std::nullopt };
  }
  query.bindValue(":user_id", user_id);
  OperationStatus error = handleQueryErrors(query);
  // NOTE: при отсутствии заявок отправляется статус NoSuchRelation
  if (error != OperationStatus::OK) {
    return { error, std::nullopt };
  }

  std::vector<unsigned int> friends;
  friends.reserve(100);
  while (query.next()) {
    unsigned int friend_id = query.value(0).toUInt();
    friends.push_back(friend_id);
  }
  if (!friends.size()) {
    return { OperationStatus::OK, std::nullopt };
  } else {
    return { OperationStatus::OK, friends };
  }
}

std::pair<OperationStatus, std::optional<std::vector<unsigned int>>>
RelationRepository::getFriendRequests(unsigned int user_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("select friend_id from relations where user_id = "
                              ":user_id and status = 'received'");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    return { OperationStatus::InternalError, std::nullopt };
  }
  query.bindValue(":user_id", user_id);
  // NOTE: при отсутствии заявок отправляется статус NoSuchRelation
  OperationStatus error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    return { error, std::nullopt };
  }

  std::vector<unsigned int> requests;
  requests.reserve(100);
  while (query.next()) {
    unsigned int friend_id = query.value(0).toUInt();
    requests.push_back(friend_id);
  }
  if (!requests.size()) {
    return { OperationStatus::OK, std::nullopt };
  } else {
    return { OperationStatus::OK, requests };
  }
}

std::pair<OperationStatus, std::optional<std::vector<unsigned int>>>
RelationRepository::getSentFriendRequests(unsigned int user_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("select friend_id from relations where user_id = "
                              ":user_id and status = 'sent'");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    return { OperationStatus::InternalError, std::nullopt };
  }
  query.bindValue(":user_id", user_id);
  OperationStatus error = handleQueryErrors(query);
  // NOTE: при отсутствии заявок отправляется статус NoSuchRelation
  if (error != OperationStatus::OK) {
    return { error, std::nullopt };
  }

  std::vector<unsigned int> requests;
  requests.reserve(100);
  while (query.next()) {
    unsigned int friend_id = query.value(0).toUInt();
    requests.push_back(friend_id);
  }
  if (!requests.size()) {
    return { OperationStatus::OK, std::nullopt };
  } else {
    return { OperationStatus::OK, requests };
  }
}

OperationStatus
RelationRepository::areFriends(unsigned int user_id, unsigned int friend_id)
{
  QSqlDatabase& connection = mConnManager->currentConnection();
  QSqlQuery query(connection);
  bool status = query.prepare("select 1 from relations where user_id = "
                              ":user_id and friend_id = :friend_id");
  if (!status) {
    qCritical(appDatabase) << query.lastError().text().toStdString();
    return OperationStatus::InternalError;
  }
  query.bindValue(":user_id", user_id);
  query.bindValue(":friend_id", friend_id);
  OperationStatus error = handleQueryErrors(query);
  if (error != OperationStatus::OK) {
    return error;
  }
  if (query.next()) {
    return OperationStatus::OK;
  } else {
    return OperationStatus::UserNotInFriends;
  }
}

OperationStatus
RelationRepository::handleQueryErrors(QSqlQuery& query)
{
  bool status = query.exec();
  int numRowsAffected = query.numRowsAffected();
  if (query.lastError().nativeErrorCode() == "23503") {
    qWarning(appDatabase) << "Foreign key violation";
    return OperationStatus::UserNotExist;
  } else if (query.lastError().nativeErrorCode() == "23505") {
    qWarning(appDatabase) << "Unique violation";
    return OperationStatus::RelationAlreadyExist;
  } else if (query.lastError().nativeErrorCode() == "23514") {
    qWarning(appDatabase) << "You can't add yourself in friends, dummy";
    return OperationStatus::RelationWithYourself;
  } else if (!numRowsAffected) {
    qWarning(appDatabase) << "No rows to update/delete";
    return OperationStatus::NoSuchRelation;
  } else if (status) {
    return OperationStatus::OK;
  } else {
    qCritical(appDatabase) << "Internal error:" << query.lastError().text();
    return OperationStatus::InternalError;
  }
}