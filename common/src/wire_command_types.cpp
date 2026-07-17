#include "wire_command_types.h"

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