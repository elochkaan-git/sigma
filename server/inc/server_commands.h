#pragma once
#include "commands.h"
#include "context.h"

#include <QString>
#include <QUuid>

/**
 * @brief Команда регистрации пользователя
 *
 */
struct RegisterUser
  : public wire::RegisterUser
  , public ClientCtx
{};

/**
 * @brief Команда входа пользователя
 *
 */
struct LoginUser
  : public wire::LoginUser
  , public ClientCtx
{};

/**
 * @brief Команда отправки сообщения
 *
 */
struct SendMessage
  : public wire::SendMessage
  , public SessionCtx
{};

/**
 * @brief Команда отправки запроса в друзья
 *
 */
struct SendFriendRequest
  : public wire::SendFriendRequest
  , public SessionCtx
{};

/**
 * @brief Команда принятия запроса в друзья
 *
 */
struct AcceptFriendRequest
  : public wire::AcceptFriendRequest
  , public SessionCtx
{};

/**
 * @brief Команда отклонения запроса в друзья
 *
 */
struct RejectFriendRequest
  : public wire::RejectFriendRequest
  , public SessionCtx
{};

/**
 * @brief Команда удаления из друзей
 *
 */
struct RemoveFriend
  : public wire::RemoveFriend
  , public SessionCtx
{};

/**
 * @brief Команда для получения списка друзей
 *
 */
struct GetFriends
  : public wire::GetFriends
  , public SessionCtx
{};

/**
 * @brief Команда для получения заявок в друзья
 *
 */
struct GetFriendRequests
  : public wire::GetFriendRequests
  , public SessionCtx
{};

/**
 * @brief Команда для получения отправленных заявок в друзья
 *
 */
struct GetSentFriendRequests
  : public wire::GetSentFriendRequests
  , public SessionCtx
{};

/**
 * @brief Команда для получения статистики с сервера - число
 * онлайн-пользователей и общее число пользователей
 */
struct GetServerStats
  : public wire::GetServerStats
  , public ClientCtx
{};

struct StartCall
  : public wire::StartCall
  , public SessionCtx
{};

struct AcceptCall
  : public wire::AcceptCall
  , public SessionCtx
{};

struct RejectCall
  : public wire::RejectCall
  , public SessionCtx
{};

struct EndCall
  : public wire::EndCall
  , public SessionCtx
{};

struct Sdp
  : public wire::Sdp
  , public SessionCtx
{};

struct IceCandidate
  : public wire::IceCandidate
  , public SessionCtx
{};