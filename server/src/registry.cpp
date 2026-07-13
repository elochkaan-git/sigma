#include "registry.h"

#include <QReadLocker>
#include <QUuid>
#include <QWriteLocker>

#include <optional>

void
OnlineUsersRegistry::registerUser(unsigned int user_id, QUuid client_id)
{
  QWriteLocker locker(&mLock);
  mOnlineUsers[user_id] = client_id;
}

void
OnlineUsersRegistry::removeUser(unsigned int user_id)
{
  QWriteLocker locker(&mLock);
  mOnlineUsers.remove(user_id);
}

std::optional<QUuid>
OnlineUsersRegistry::getClientId(unsigned int user_id)
{
  QReadLocker locker(&mLock);
  if (!mOnlineUsers.contains(user_id))
    return std::nullopt;
  return mOnlineUsers.value(user_id);
}