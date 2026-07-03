#include "repositories.h"
#include "connection_manager.h"
#include "statuses.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

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