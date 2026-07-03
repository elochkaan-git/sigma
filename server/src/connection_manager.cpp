#include "connection_manager.h"
#include <QThread>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <cstdlib>
#include <qobject.h>
#include <stdexcept>
#include <string>

ConnectionManager::ConnectionManager(const DatabaseInfo& db_info)
{
  this->mDatabase = db_info;
}

QSqlDatabase&
ConnectionManager::currentConnection()
{
  if (!mConnections.hasLocalData()) {
    QSqlDatabase newConnection = QSqlDatabase::addDatabase(
      "QPSQL",
      QString::fromStdString("thread" + std::to_string(mThreadCounter++)));
    newConnection.setHostName(
      mDatabase.hostName); // FIXME: Change when integrate with docker
    newConnection.setDatabaseName(
      mDatabase.databaseName); // FIXME: Change when integrate with docker
    newConnection.setUserName(
      mDatabase.userName); // FIXME: Change when integrate with docker
    newConnection.setPassword(
      mDatabase.password); // FIXME: Change when integrate with docker
    mConnections.setLocalData(newConnection);
  }
  QSqlDatabase& connection = mConnections.localData();
  if (connection.isOpen()) {
    return connection;
  } else {
    bool status = connection.open();
    if (!status)
      throw std::runtime_error(
        connection.lastError()
          .text()
          .toStdString()); // TODO: Replace throwing error on returning warning
    return connection;
  }
}