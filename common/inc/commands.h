#pragma once
#include <QString>
#include <QUuid>

namespace wire {

/**
 * @brief Команда регистрации пользователя
 *
 */
struct RegisterUser
{
  QString login;
  QString pwd;
};

/**
 * @brief Команда входа пользователя
 *
 */
struct LoginUser
{
  QString login;
  QString pwd;
};

/**
 * @brief Команда отправки сообщения
 *
 */
struct SendMessage
{
  unsigned int receiver_id;
  QString content;
};

/**
 * @brief Команда отправки запроса в друзья
 *
 */
struct SendFriendRequest
{
  unsigned int friend_id;
};

/**
 * @brief Команда принятия запроса в друзья
 *
 */
struct AcceptFriendRequest
{
  unsigned int friend_id;
};

/**
 * @brief Команда отклонения запроса в друзья
 *
 */
struct RejectFriendRequest
{
  unsigned int friend_id;
};

/**
 * @brief Команда удаления из друзей
 *
 */
struct RemoveFriend
{
  unsigned int friend_id;
};

/**
 * @brief Команда для получения списка друзей
 *
 */
struct GetFriends
{
};

/**
 * @brief Команда для получения заявок в друзья
 *
 */
struct GetFriendRequests
{
};

/**
 * @brief Команда для получения отправленных заявок в друзья
 *
 */
struct GetSentFriendRequests
{
};

/**
 * @brief Команда для получения статистики с сервера - число
 * онлайн-пользователей и общее число пользователей
 */
struct GetServerStats
{
};

struct StartCall
{
  unsigned int callee_id;
  bool with_video;
};

struct AcceptCall
{
  QUuid call_id;
};

struct RejectCall
{
  QUuid call_id;
};

struct EndCall
{
  QUuid call_id;
};

struct Sdp
{
  QUuid call_id;
  QString sdp;
};

struct IceCandidate
{
  QUuid call_id;
  QString candidate;
  QString mid;
};

struct GetTurnCredentials
{
};

}