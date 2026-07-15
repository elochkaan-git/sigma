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

/**
 * @brief Команда отправки запроса в друзья
 *
 */
struct SendFriendRequest
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

/**
 * @brief Команда принятия запроса в друзья
 *
 */
struct AcceptFriendRequest
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

/**
 * @brief Команда отклонения запроса в друзья
 *
 */
struct RejectFriendRequest
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

/**
 * @brief Команда удаления из друзей
 *
 */
struct RemoveFriend
{
  QUuid client_id;
  unsigned int user_id;
  unsigned int friend_id;
};

/**
 * @brief Команда для получения списка друзей
 *
 */
struct GetFriends
{
  QUuid client_id;
  unsigned int user_id;
};

/**
 * @brief Команда для получения заявок в друзья
 *
 */
struct GetFriendRequests
{
  QUuid client_id;
  unsigned int user_id;
};

/**
 * @brief Команда для получения отправленных заявок в друзья
 *
 */
struct GetSentFriendRequests
{
  QUuid client_id;
  unsigned int user_id;
};