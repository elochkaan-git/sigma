#include "registry.h"

#include <QReadLocker>
#include <QUuid>
#include <QWriteLocker>

#include <optional>

bool
OnlineUsersRegistry::registerUser(unsigned int user_id, const QUuid& client_id)
{
  QWriteLocker locker(&mLock);
  if (mOnlineUsers.contains(user_id)) {
    return false;
  }
  mOnlineUsers[user_id] = client_id;
  return true; 
}

bool
OnlineUsersRegistry::removeUser(unsigned int user_id)
{
  QWriteLocker locker(&mLock);
  return mOnlineUsers.remove(user_id);
}

std::optional<QUuid>
OnlineUsersRegistry::getClientId(unsigned int user_id)
{
  QReadLocker locker(&mLock);
  auto it = mOnlineUsers.find(user_id);
  if (it != mOnlineUsers.end()) {
    return it.value();
  }
  return std::nullopt;
}

unsigned int
OnlineUsersRegistry::totalOnline()
{
  QReadLocker locker(&mLock);
  return mOnlineUsers.size();
}