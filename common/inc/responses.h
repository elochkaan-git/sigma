#pragma once
#include "structures.h"

#include <QString>
#include <QUuid>

namespace wire {

/**
 * @brief Ответ на регистрацию пользователя
 * @see RegisterUser
 * @see OperationStatus
 */
struct RegisterUserResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на вход пользователя
 * @see LoginUser
 * @see OperationStatus
 */
struct LoginUserResponse
{
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
  OperationStatus status;
};

/**
 * @brief Ответ на новое письмо пользователю. Отправляется, когда адресат
 * находится в сети
 * @see OperationStatus
 */
struct NewMessageResponse
{
  unsigned int sender_id;
  QString content;
};

/**
 * @brief Ответ на отправку заявки в друзья
 */
struct SendFriendRequestResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на принятие заявки в друзья
 */
struct AcceptFriendRequestResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на отклонение заявки в друзья
 */
struct RejectFriendRequestResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на удаление из друзей
 */
struct RemoveFriendResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на получение списка друзей
 */
struct GetFriendsResponse
{
  OperationStatus status;
  std::optional<std::vector<User>>
    friends; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение списка заявок в друзья
 */
struct GetFriendRequestsResponse
{
  OperationStatus status;
  std::optional<std::vector<User>>
    requests; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение списка отправленных заявок в друзья
 */
struct GetSentFriendRequestsResponse
{
  OperationStatus status;
  std::optional<std::vector<User>>
    sent_requests; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение статистики с сервера - число
 * онлайн-пользователей и общее число пользователей
 */
struct GetServerStatsResponse
{
  OperationStatus status;
  unsigned int online;
  unsigned int total;
};

}