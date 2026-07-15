#pragma once
#include <QString>
#include <QUuid>

/**
 * @brief Команда регистрации пользователя
 *
 */
struct RegisterUser
{
  QUuid client_id;
  QString login;
  QString pwd;
};

/**
 * @brief Команда входа пользователя
 *
 */
struct LoginUser
{
  QUuid client_id;
  QString login;
  QString pwd;
};

/**
 * @brief Команда отправки сообщения
 *
 */
struct SendMessage
{
  QUuid client_id;
  unsigned int sender_id;
  unsigned int receiver_id;
  QString content;
};

struct SendFriendRequest
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

struct AcceptFriendRequest
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

struct RejectFriendRequest
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

struct RemoveFriend
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

struct GetFriends
{
  QUuid client_id;
  unsigned int user_id;
};

struct GetFriendRequests
{
  QUuid client_id;
  unsigned int user_id;
};

struct GetSentFriendRequests
{
  QUuid client_id;
  unsigned int user_id;
};