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
    qCritical(appService) << "Out of memory while computing hash";
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
  auto [status, pwd_hash] = mUserRepo->getUserPwdHash(login);
  if (status == OperationStatus::OK) {
    std::string s_pwd = passwd.toStdString();
    std::string s_pwd_hash = pwd_hash.value().toStdString();
    if (crypto_pwhash_str_verify(
          s_pwd_hash.c_str(), s_pwd.c_str(), s_pwd.length()) != 0) {
      qWarning(appService) << QString("Wrong password, %1").arg(login);
      return { OperationStatus::InvalidCredentials, std::nullopt };
    }
    const auto [status, user_info] = mUserRepo->getUserByLogin(login);
    if (status != OperationStatus::OK) {
      return { status, std::nullopt };
    }
    return { OperationStatus::OK, user_info.value().user_id };
  } else if (status == OperationStatus::UserNotExist) {
    qWarning(appService) << QString("User %1 not exists").arg(login);
    return { OperationStatus::UserNotExist, std::nullopt };
  } else {
    qCritical(appService) << QString("Internal error");
    return { OperationStatus::InternalError, std::nullopt };
  }
}

std::pair<OperationStatus, std::optional<User>>
UserService::getUserByID(unsigned int user_id)
{
  return this->mUserRepo->getUserByID(user_id);
}

std::pair<OperationStatus, unsigned int>
UserService::countUsers()
{
  return this->mUserRepo->countUsers();
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
MessageService::deleteFromQueue(const std::vector<unsigned int>& msg_ids)
{
  return mMsgRepo->deleteFromQueue(msg_ids);
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
  return this->getUsers(user_id, "friends");
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getFriendRequests(unsigned int user_id)
{
  return this->getUsers(user_id, "received");
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getSentFriendRequests(unsigned int user_id)
{
  return this->getUsers(user_id, "sent");
}

OperationStatus
RelationService::areFriends(unsigned int user_id, unsigned int friend_id)
{
  return this->mRelRepo->areFriends(user_id, friend_id);
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getUsers(unsigned int user_id, QString user_status)
{
  std::pair<OperationStatus, std::optional<std::vector<unsigned int>>> ids;
  if (user_status == "friends") {
    ids = this->mRelRepo->getFriendsID(user_id);
  } else if (user_status == "received") {
    ids = this->mRelRepo->getFriendRequests(user_id);
  } else if (user_status == "sent") {
    ids = this->mRelRepo->getSentFriendRequests(user_id);
  }
  OperationStatus status{ ids.first };
  const auto users_ids{ ids.second };

  if (status != OperationStatus::OK || !users_ids.has_value()) {
    qWarning(appService)
      << QString("User %1 have no '%2' or have error %3")
           .arg(user_id)
           .arg(user_status)
           .arg((int)status);
    return { status, std::nullopt };
  }

  std::pair<OperationStatus, std::optional<std::vector<User>>> users_pair;
  users_pair = this->mUserRepo->getUsersById(users_ids.value());
  status = users_pair.first;
  const auto users{ users_pair.second };

  if (status != OperationStatus::OK || !users.has_value()) {
    qWarning(appService)
      << QString("No users with such ids or error %1").arg((int)status);
    return { status, std::nullopt };
  }
  return { OperationStatus::OK, users };
}