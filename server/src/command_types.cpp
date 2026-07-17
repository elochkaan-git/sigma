#include "command_types.h"

CommandType
getTypeOfCommand(const Command& cmd)
{
  return std::visit(
    overloaded{
      [](const Error&) { return CommandType::ERROR; },
      [](const RegisterUser&) { return CommandType::REGISTER; },
      [](const LoginUser&) { return CommandType::LOGIN; },
      [](const SendMessage&) { return CommandType::SEND_MESSAGE; },
      [](const SendFriendRequest&) { return CommandType::SEND_FRIEND_REQUEST; },
      [](const AcceptFriendRequest&) {
        return CommandType::ACCEPT_FRIEND_REQUEST;
      },
      [](const RejectFriendRequest&) {
        return CommandType::REJECT_FRIEND_REQUEST;
      },
      [](const RemoveFriend&) { return CommandType::REMOVE_FRIEND; },
      [](const GetFriends&) { return CommandType::GET_FRIENDS; },
      [](const GetFriendRequests&) { return CommandType::GET_FRIEND_REQUESTS; },
      [](const GetSentFriendRequests&) {
        return CommandType::GET_SENT_FRIEND_REQUESTS;
      },
      [](const GetServerStats&) { return CommandType::GET_SERVER_STATS; } },
    cmd);
}