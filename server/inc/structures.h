#pragma once
#include <QString>

/**
 * @brief Enum-класс статусов выполнения операций в репозиториях
 *
 * @see UserRepository
 */
enum class OperationStatus
{
  OK = 0,        /**< Все в порядке, операция завершилась без ошибок */
  UserExist = 1, /**< Если при регистрации попытаться указать уже
                   зарегистрированный логин */
  UserNotExist = 2,
  InvalidCredentials = 3,
  InternalError = 255
};

/**
 * @brief Структура письма из очереди
 * @see MessageRepository::getQueuedMessages
 */
struct Message
{
  unsigned int msg_id;
  unsigned int sender_id;
  QString content;
};

/**
 * @brief Данные пользователя
 * @see UserRepository::findUserByLogin
 */
struct UserCredentials
{
  unsigned int user_id;
  QString login;
  QString pwd_hash;
};