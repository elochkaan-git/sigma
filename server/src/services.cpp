#include "services.h"
#include "logging.h"
#include "repositories.h"
#include "sodium/crypto_pwhash.h"
#include "structures.h"
#include <QString>
#include <optional>
#include <sodium.h>
#include <stdexcept>
#include <string>
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
  if (crypto_pwhash_str(pwd_hash_c,
                        pwd_string.c_str(),
                        pwd_string.length(),
                        crypto_pwhash_OPSLIMIT_INTERACTIVE,
                        crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
    throw std::runtime_error("Out of memory");
  }
  QString pwd_hash(pwd_hash_c);
  return mUserRepo->registerUser(login, pwd_hash);
}

std::pair<OperationStatus, std::optional<unsigned int>>
UserService::loginUser(QString login, QString passwd)
{
  auto [status, user_info] = mUserRepo->getUserByLogin(login);
  if (status == OperationStatus::UserNotExist) {
    return { OperationStatus::UserNotExist, std::nullopt };
  } else {
    std::string pwd = passwd.toStdString();
    if (crypto_pwhash_str_verify(
          user_info.value().pwd_hash.toStdString().c_str(),
          pwd.c_str(),
          pwd.length()) != 0) {
      return { OperationStatus::InvalidCredentials, std::nullopt };
    }
    return { OperationStatus::OK, user_info.value().user_id };
  }
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
  const auto [status, ids] = this->mRelRepo->getFriends(user_id);
  if (status != OperationStatus::OK || !ids.has_value()) {
    return { status, std::nullopt };
  }
  std::vector<User> friends;
  friends.reserve(ids.value().size());
  for (const auto& id : ids.value()) {
    // TODO: добавить метод получения списка пользователей, а не по одному
    const auto [user_status, user] = mUserRepo->getUserByID(id);
    if (user_status != OperationStatus::OK || !user.has_value()) {
      qWarning(appService) << "Friend id" << id << "not found, skipping";
      continue;
    }
    friends.push_back({ user.value().user_id, user.value().login, "" });
  }
  return { OperationStatus::OK, friends };
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getFriendRequests(unsigned int user_id)
{
  const auto [status, ids] = this->mRelRepo->getFriendRequests(user_id);
  if (status != OperationStatus::OK || !ids.has_value()) {
    return { status, std::nullopt };
  }
  std::vector<User> requests;
  requests.reserve(ids.value().size());
  for (const auto& id : ids.value()) {
    // TODO: добавить метод получения списка пользователей, а не по одному
    const auto [user_status, user] = mUserRepo->getUserByID(id);
    if (user_status != OperationStatus::OK || !user.has_value()) {
      qWarning(appService) << "Friend id" << id << "not found, skipping";
      continue;
    }
    requests.push_back({ user.value().user_id, user.value().login, "" });
  }
  return { OperationStatus::OK, requests };
}

std::pair<OperationStatus, std::optional<std::vector<User>>>
RelationService::getSentFriendRequests(unsigned int user_id)
{
  const auto [status, ids] = this->mRelRepo->getSentFriendRequests(user_id);
  if (status != OperationStatus::OK || !ids.has_value()) {
    return { status, std::nullopt };
  }
  std::vector<User> sentRequests;
  sentRequests.reserve(ids.value().size());
  for (const auto& id : ids.value()) {
    // TODO: добавить метод получения списка пользователей, а не по одному
    const auto [user_status, user] = mUserRepo->getUserByID(id);
    if (user_status != OperationStatus::OK || !user.has_value()) {
      qWarning(appService) << "Friend id" << id << "not found, skipping";
      continue;
    }
    sentRequests.push_back({ user.value().user_id, user.value().login, "" });
  }
  return { OperationStatus::OK, sentRequests };
}