#include "connection_manager.h"

#include "logging.h"

#include <QObject>
#include <QThread>
#include <QtLogging>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

#include <cstdlib>
#include <stdexcept>
#include <string>

ConnectionManager::ConnectionManager(const DatabaseInfo& db_info)
{
  if (!checkDatabaseInfo(db_info)) {
    throw std::runtime_error("Invalid DatabaseInfo structure");
  }
  this->mDatabase = db_info;
}

QSqlDatabase&
ConnectionManager::currentConnection()
{
  if (!mConnections.hasLocalData()) {
    QString connName = QString::fromStdString("thread" + std::to_string(mThreadCounter++));

    QSqlDatabase newConnection = QSqlDatabase::addDatabase("QPSQL", connName);
    newConnection.setHostName(mDatabase.hostName);
    newConnection.setDatabaseName(mDatabase.databaseName);
    newConnection.setUserName(mDatabase.userName);
    newConnection.setPassword(mDatabase.password);

    if (QThread* thread = QThread::currentThread()) {
      QObject::connect(thread, &QThread::finished, thread, [connName]() {
        {
          QSqlDatabase db = QSqlDatabase::database(connName, false);
          if (db.isOpen()) {
            db.close();
          }
        }
        QSqlDatabase::removeDatabase(connName);
      });
    }

    mConnections.setLocalData(new QSqlDatabase(newConnection));
  }
  
  QSqlDatabase& connection = *mConnections.localData();
  if (connection.isOpen()) {
    return connection;
  }
  
  bool status = connection.open();
  if (!status) {
    qCritical(appDatabase) << connection.lastError().text();
    throw std::runtime_error("Can't connect to database");
  }
  return connection;
}

bool
ConnectionManager::checkDatabaseInfo(const DatabaseInfo& db_info)
{
  if (db_info.hostName.isEmpty()) {
    qCritical(appDatabase) << "Host name is not provided";
    return false;
  } else if (db_info.databaseName.isEmpty()) {
    qCritical(appDatabase) << "Database name is not provided";
    return false;
  } else if (db_info.userName.isEmpty()) {
    qCritical(appDatabase) << "User name is not provided";
    return false;
  } else if (db_info.password.isEmpty()) {
    qCritical(appDatabase) << "Password is not provided";
    return false;
  }
  return true;
}