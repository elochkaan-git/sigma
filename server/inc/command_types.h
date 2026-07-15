#pragma once
#include "commands.h"
#include "responses.h"
#include "structures.h"

#include <variant>

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

/**
 * @brief Типы команд
 */
enum class CommandType
{
  ERROR,
  REGISTER,
  LOGIN,
  SEND_MESSAGE,
  SEND_FRIEND_REQUEST,
  ACCEPT_FRIEND_REQUEST,
  REJECT_FRIEND_REQUEST,
  REMOVE_FRIEND,
  GET_FRIENDS,
  GET_FRIEND_REQUESTS,
  GET_SENT_FRIEND_REQUESTS,
  OVERSIZED
};

/**
 * @brief сокращение для варианта ответов
 */
using Response = std::variant<Error,
                              RegisterUserResponse,
                              LoginUserResponse,
                              NewMessageResponse,
                              SendMessageResponse,
                              SendFriendRequestResponse,
                              AcceptFriendRequestResponse,
                              RejectFriendRequestResponse,
                              RemoveFriendResponse,
                              GetFriendsResponse,
                              GetFriendRequestsResponse,
                              GetSentFriendRequestsResponse>;
/**
 * @brief сокращение для варианта команд
 */
using Command = std::variant<Error,
                             RegisterUser,
                             LoginUser,
                             SendMessage,
                             SendFriendRequest,
                             AcceptFriendRequest,
                             RejectFriendRequest,
                             RemoveFriend,
                             GetFriends,
                             GetFriendRequests,
                             GetSentFriendRequests>;

/**
 * @brief Возвращает тип команды
 *
 * @param cmd команда
 * @return CommandType
 */
CommandType
getTypeOfCommand(const Command& cmd);

/**
 * @brief Превращает enum в QString
 *
 * @param cmd_type тип команды
 * @return QString команда в виде строки
 */
QString
commandTypeToString(const CommandType& cmd_type);
