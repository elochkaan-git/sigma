#pragma once
#include "commands.h"

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
  GET_SERVER_STATS,
  START_CALL,
  ACCEPT_CALL,
  REJECT_CALL,
  END_CALL,
  SDP,
  ICE_CANDIDATE,
  GET_TURN_CREDENTIALS,
  OVERSIZED
};

/**
 * @brief Превращает enum в QString
 *
 * @param cmd_type тип команды
 * @return QString команда в виде строки
 */
QString
commandTypeToString(const CommandType& cmd_type);
