#pragma once
#include "connection_manager.h"
#include "statuses.h"
#include <QString>

class UserRepository
{
public:
  UserRepository(ConnectionManager* manager);
  OperationStatus registerUser(QString login, QString pwd_hash);

private:
  ConnectionManager* mConnManager;
};