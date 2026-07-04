#include "services.h"
#include "repositories.h"
#include "sodium/crypto_pwhash.h"
#include "structures.h"
#include <QString>
#include <optional>
#include <sodium.h>
#include <stdexcept>
#include <string>

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

std::optional<unsigned int>
UserService::loginUser(QString login, QString passwd)
{
  std::optional<UserCredentials> user_info = mUserRepo->findUserByLogin(login);
  if (!user_info.has_value()) {
    return std::nullopt;
  } else {
    std::string pwd = passwd.toStdString();
    if (crypto_pwhash_str_verify(
          user_info.value().pwd_hash.toStdString().c_str(),
          pwd.c_str(),
          pwd.length()) != 0) {
      return std::nullopt;
    }
    return user_info.value().user_id;
  }
}

MessageService::MessageService(MessageRepository* repo)
  : mMsgRepo(repo)
{
}

void
MessageService::saveToQueue(unsigned int sender_id,
                            unsigned int receiver_id,
                            QString content)
{
  mMsgRepo->saveToQueue(sender_id, receiver_id, content);
}

std::vector<Message>
MessageService::getQueuedMessages(unsigned int user_id)
{
  return mMsgRepo->getQueuedMessages(user_id);
}

void
MessageService::deleteFromQueue(unsigned int msg_id)
{
  mMsgRepo->deleteFromQueue(msg_id);
}