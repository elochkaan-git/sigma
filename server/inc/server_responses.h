#pragma once
#include "context.h"
#include "responses.h"

#include <QString>
#include <QUuid>

/**
 * @brief Ответ на регистрацию пользователя
 * @see RegisterUser
 * @see OperationStatus
 */
struct RegisterUserResponse
  : public wire::RegisterUserResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на вход пользователя
 * @see LoginUser
 * @see OperationStatus
 */
struct LoginUserResponse
  : public wire::LoginUserResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на отправленное письмо. Нужно для понимания, доставлено письмо
 * вообще или нет
 * @see SendMessage
 * @see OperationStatus
 */
struct SendMessageResponse
  : public wire::SendMessageResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на новое письмо пользователю. Отправляется, когда адресат
 * находится в сети
 * @see OperationStatus
 */
struct NewMessageResponse
  : public wire::NewMessageResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на отправку заявки в друзья
 */
struct SendFriendRequestResponse
  : public wire::SendFriendRequestResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на принятие заявки в друзья
 */
struct AcceptFriendRequestResponse
  : public wire::AcceptFriendRequestResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на отклонение заявки в друзья
 */
struct RejectFriendRequestResponse
  : public wire::RejectFriendRequestResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на удаление из друзей
 */
struct RemoveFriendResponse
  : public wire::RemoveFriendResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение списка друзей
 */
struct GetFriendsResponse
  : public wire::GetFriendsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение списка заявок в друзья
 */
struct GetFriendRequestsResponse
  : public wire::GetFriendRequestsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение списка отправленных заявок в друзья
 */
struct GetSentFriendRequestsResponse
  : public wire::GetSentFriendRequestsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение статистики с сервера - число
 * онлайн-пользователей и общее число пользователей
 */
struct GetServerStatsResponse
  : public wire::GetServerStatsResponse
  , public ClientCtx
{};