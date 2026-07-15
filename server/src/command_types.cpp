#include "command_types.h"
#include "commands.h"

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

QString
commandTypeToString(const CommandType& cmd_type)
{
  switch (cmd_type) {
    case CommandType::SEND_FRIEND_REQUEST:
      return "SendFriendRequest";
    case CommandType::ACCEPT_FRIEND_REQUEST:
      return "AcceptFriendRequest";
    case CommandType::REJECT_FRIEND_REQUEST:
      return "RejectFriendRequest";
    case CommandType::REMOVE_FRIEND:
      return "RemoveFriend";
    case CommandType::GET_FRIENDS:
      return "GetFriend";
    case CommandType::GET_FRIEND_REQUESTS:
      return "GetFriendRequests";
    case CommandType::GET_SENT_FRIEND_REQUESTS:
      return "GetSentFriendRequests";
    case CommandType::GET_SERVER_STATS:
      return "GetServerStats";
    case CommandType::REGISTER:
      return "RegisterUser";
    case CommandType::LOGIN:
      return "LoginUser";
    case CommandType::SEND_MESSAGE:
      return "SendMessage";
    default:
      return "Error";
  }
}