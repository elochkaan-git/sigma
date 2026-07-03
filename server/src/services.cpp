#include "services.h"
#include "repositories.h"
#include "sodium/crypto_pwhash.h"
#include "statuses.h"
#include <QString>
#include <sodium.h>
#include <stdexcept>

UserService::UserService(UserRepository* repo)
  : mRepo(repo)
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
  return mRepo->registerUser(login, pwd_hash);
}