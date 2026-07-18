#pragma once
#include <QDateTime>
#include <QString>
#include <QUuid>

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
  RelationAlreadyExist = 4,
  RelationWithYourself = 5,
  NoSuchRelation = 6,
  UserNotInFriends = 7,
  UserOffline = 8,       /**< Пользователь не в сети (например, попытка позвонить
                           оффлайн-пользователю) */
  NoSuchCall = 9,        /**< Звонка с таким call_id не существует (уже завершен
                           или никогда не существовал) */
  NotCallParticipant = 10, /**< Пользователь не является участником данного
                             звонка (не caller и не callee) */
  CallWithYourself = 11, /**< Попытка позвонить самому себе */
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
 * @see UserRepository::getUserByLogin
 */
struct User
{
  unsigned int user_id;
  QString login;
};

namespace wire {

/**
 * @brief Универсальная структура ошибки для случаев, не заложенных в
 * OperationStatus. Используется и как команда, и как ответ.
 *
 */
struct Error
{
  QString reason;  /**< причина ошибки */
};

}