#pragma once
#include <QHash>
#include <QReadWriteLock>
#include <QString>
#include <QUuid>
#include <optional>

class OnlineUsersRegistry
{
public:
  void registerUser(unsigned int user_id, QUuid client_id);
  void removeUser(unsigned int user_id);
  std::optional<QUuid> getClientId(unsigned int user_id);

private:
  QHash<unsigned int, QUuid> mOnlineUsers;
  QReadWriteLock mLock;
};