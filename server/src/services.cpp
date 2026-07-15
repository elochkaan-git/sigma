#include "services.h"

#include "logging.h"
#include "repositories.h"
#include "structures.h"

#include <QString>
#include <qlogging.h>
#include <sodium.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

UserService::UserService(UserRepository* u_repo)
  : mUserRepo(u_repo)
{
}

OperationStatus
UserService::registerUser(QString login, QString passwd)
{
  char pwd_hash_c[crypto_pwhash_STRBYTES];
  std::string pwd_string = passwd.toStdString();
  qInfo(appService) << QString("Computing password hash of %1").arg(login);
  if (crypto_pwhash_str(pwd_hash_c,
                        pwd_string.c_str(),
                        pwd_string.length(),
                        crypto_pwhash_OPSLIMIT_INTERACTIVE,
                        crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
    qFatal(appService) << "Out of memory while computing hash";
    throw std::runtime_error("Out of memory");
  } else {
    qInfo(appService) << QString("Password hash of %1 computed").arg(login);
  }
  QString pwd_hash(pwd_hash_c);
  return mUserRepo->registerUser(login, pwd_hash);
}

std::pair<OperationStatus, std::optional<unsigned int>>
UserService::loginUser(QString login, QString passwd)
{
  auto [status, user_info] = mUserRepo->getUserByLogin(login);
  if (status == OperationStatus::UserNotExist) {
    qWarning(appService) << QString("User %1 not exists").arg(login);
    return { OperationStatus::UserNotExist, std::nullopt };
  } else {
    std::string pwd = passwd.toStdString();
    if (crypto_pwhash_str_verify(
          user_info.value().pwd_hash.toStdString().c_str(),
          pwd.c_str(),
          pwd.length()) != 0) {
      qWarning(appService) << QString("Wrong password, %1").arg(login);
      return { OperationStatus::InvalidCredentials, std::nullopt };
    }
    return { OperationStatus::OK, user_info.value().user_id };
  }
}

std::pair<OperationStatus, std::optional<User>>
UserService::getUserByID(unsigned int user_id)
{
  return this->mUserRepo->getUserByID(user_id);
}

MessageService::MessageService(MessageRepository* repo)
  : mMsgRepo(repo)
{
}

OperationStatus
MessageService::saveToQueue(unsigned int sender_id,
                            unsigned int receiver_id,
                            QString content)
{
  return mMsgRepo->saveToQueue(sender_id, receiver_id, content);
}

std::pair<OperationStatus, std::optional<std::vector<Message>>>
MessageService::getQueuedMessages(unsigned int user_id)
{
  return mMsgRepo->getQueuedMessages(user_id);
}

OperationStatus
MessageService::deleteFromQueue(unsigned int msg_id)
{
  return mMsgRepo->deleteFromQueue(msg_id);
}

RelationService::RelationService(RelationRepository* rel_repo,
                                 UserRepository* u_repo)
  : mRelRepo(rel_repo)
  , mUserRepo(u_repo)
{
}

OperationStatus
RelationService::sendFriendRequest(unsigned int user_id, unsigned int friend_id)
{
  return this->mRelRepo->sendFriendRequest(user_id, friend_id);
}

OperationStatus
RelationService::acceptFriendRequest(unsigned int user_id,
                                     unsigned int friend_id)
{
  return this->mRelRepo->acceptFriendRequest(user_id, friend_id);
}

OperationStatus
RelationService::rejectFriendRequest(unsigned int user_id,
                                     unsigned int friend_id)
{
  return this->mRelRepo->rejectFriendRequest(user_id, friend_id);
}

OperationStatus
RelationService::removeFriend(unsigned int user_id, unsigned int friend_id)
{
  return this->mRelRepo->removeFriend(user_id, friend_id);
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getFriends(unsigned int user_id)
{
  const auto [status_users, friends] = this->mRelRepo->getFriendsID(user_id);
  if (status_users != OperationStatus::OK || !friends.has_value()) {
    qWarning(appService) << QString("User %1 have no friends or have error %2")
                              .arg(user_id)
                              .arg((int)status_users);
    return { status_users, std::nullopt };
  }
  auto [status_rel, users] = this->mUserRepo->getUsersById(friends.value());
  if (status_rel != OperationStatus::OK || !users.has_value()) {
    qWarning(appService)
      << QString("No users with such ids or error %1").arg((int)status_users);
    return { status_rel, std::nullopt };
  }
  for (auto& u : users.value()) {
    u.pwd_hash.clear();
  }
  return { OperationStatus::OK, users };
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getFriendRequests(unsigned int user_id)
{
  const auto [status_users, requests] =
    this->mRelRepo->getFriendRequests(user_id);
  if (status_users != OperationStatus::OK || !requests.has_value()) {
    qWarning(appService)
      << QString("User %1 have no friend requests or have error %2")
           .arg(user_id)
           .arg((int)status_users);
    return { status_users, std::nullopt };
  }
  auto [status_rel, users] = this->mUserRepo->getUsersById(requests.value());
  if (status_rel != OperationStatus::OK || !users.has_value()) {
    qWarning(appService)
      << QString("No users with such ids or error %1").arg((int)status_users);
    return { status_rel, std::nullopt };
  }
  for (auto& u : users.value()) {
    u.pwd_hash.clear();
  }
  return { OperationStatus::OK, users };
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getSentFriendRequests(unsigned int user_id)
{
  const auto [status_users, sentRequests] =
    this->mRelRepo->getSentFriendRequests(user_id);
  if (status_users != OperationStatus::OK || !sentRequests.has_value()) {
    qWarning(appService)
      << QString("User %1 have no sent friend requests or have error %2")
           .arg(user_id)
           .arg((int)status_users);
    return { status_users, std::nullopt };
  }
  auto [status_rel, users] =
    this->mUserRepo->getUsersById(sentRequests.value());
  if (status_rel != OperationStatus::OK || !users.has_value()) {
    qWarning(appService)
      << QString("No users with such ids or error %1").arg((int)status_users);
    return { status_rel, std::nullopt };
  }
  for (auto& u : users.value()) {
    u.pwd_hash.clear();
  }
  return { OperationStatus::OK, users };
}

OperationStatus
RelationService::areFriends(unsigned int user_id, unsigned int friend_id)
{
  return this->mRelRepo->areFriends(user_id, friend_id);
}