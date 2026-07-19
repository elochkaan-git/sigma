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
    case CommandType::START_CALL:
      return "StartCall";
    case CommandType::ACCEPT_CALL:
      return "AcceptCall";
    case CommandType::REJECT_CALL:
      return "RejectCall";
    case CommandType::END_CALL:
      return "EndCall";
    case CommandType::SDP:
      return "Sdp";
    case CommandType::ICE_CANDIDATE:
      return "IceCandidate";
    case CommandType::REGISTER:
      return "RegisterUser";
    case CommandType::LOGIN:
      return "LoginUser";
    case CommandType::SEND_MESSAGE:
      return "SendMessage";
    case CommandType::GET_TURN_CREDENTIALS:
      return "GetTurnCredentials";
    case CommandType::SET_AVATAR:
      return "SetAvatar";
    case CommandType::GET_ONLINE_USERS:
      return "GetOnlineUsers";
    default:
      return "Error";
  }
}