#pragma once
#include "structures.h"
#include <QString>
#include <QUuid>

/**
 * @brief Ответ на регистрацию пользователя
 * @see RegisterUser
 * @see OperationStatus
 */
struct RegisterUserResponse
{
  QUuid client_id;
  OperationStatus status;
};

/**
 * @brief Ответ на вход пользователя
 * @see LoginUser
 * @see OperationStatus
 */
struct LoginUserResponse
{
  QUuid client_id;
  unsigned int user_id;
  OperationStatus status;
};

/**
 * @brief Ответ на отправленное письмо. Нужно для понимания, доставлено письмо
 * вообще или нет
 * @see SendMessage
 * @see OperationStatus
 */
struct SendMessageResponse
{
  QUuid client_id;
  OperationStatus status;
};

/**
 * @brief Ответ на новое письмо пользователю. Отправляется, когда адресат
 * находится в сети
 * @see OperationStatus
 */
struct NewMessageResponse
{
  QUuid client_id; // Получатель
  unsigned int sender_id;
  QString content;
};

struct SendFriendRequestResponse
{
  QUuid client_id;
  OperationStatus status;
};

struct AcceptFriendRequestResponse
{
  QUuid client_id;
  OperationStatus status;
};

struct RejectFriendRequestResponse
{
  QUuid client_id;
  OperationStatus status;
};

struct RemoveFriendResponse
{
  QUuid client_id;
  OperationStatus status;
};