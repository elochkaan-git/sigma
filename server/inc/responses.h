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

/**
 * @brief Ответ на отправку заявки в друзья
 */
struct SendFriendRequestResponse
{
  QUuid client_id;
  OperationStatus status;
};

/**
 * @brief Ответ на принятие заявки в друзья
 */
struct AcceptFriendRequestResponse
{
  QUuid client_id;
  OperationStatus status;
};

/**
 * @brief Ответ на отклонение заявки в друзья
 */
struct RejectFriendRequestResponse
{
  QUuid client_id;
  OperationStatus status;
};

/**
 * @brief Ответ на удаление из друзей
 */
struct RemoveFriendResponse
{
  QUuid client_id;
  OperationStatus status;
};

/**
 * @brief Ответ на получение списка друзей
 */
struct GetFriendsResponse
{
  QUuid client_id;
  OperationStatus status;
  std::optional<std::vector<User>>
    friends; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение списка заявок в друзья
 */
struct GetFriendRequestsResponse
{
  QUuid client_id;
  OperationStatus status;
  std::optional<std::vector<User>>
    requests; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение списка отправленных заявок в друзья
 */
struct GetSentFriendRequestsResponse
{
  QUuid client_id;
  OperationStatus status;
  std::optional<std::vector<User>>
    sentRequests; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение статистики с сервера - число
 * онлайн-пользователей и общее число пользователей
 */
struct GetServerStatsResponse
{
  QUuid client_id;
  OperationStatus status;
  unsigned int online;
  unsigned int total;
};