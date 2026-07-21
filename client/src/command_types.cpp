#include "command_types.h"
#include "commands.h"

CommandType
getTypeOfCommand(const Command& cmd)
{
  return std::visit(
    overloaded{
      [](const wire::Error&) { return CommandType::ERROR; },
      [](const wire::RegisterUser&) { return CommandType::REGISTER; },
      [](const wire::LoginUser&) { return CommandType::LOGIN; },
      [](const wire::SendMessage&) { return CommandType::SEND_MESSAGE; },
      [](const wire::SendFriendRequest&) { return CommandType::SEND_FRIEND_REQUEST; },
      [](const wire::AcceptFriendRequest&) {
        return CommandType::ACCEPT_FRIEND_REQUEST;
      },
      [](const wire::RejectFriendRequest&) {
        return CommandType::REJECT_FRIEND_REQUEST;
      },
      [](const wire::RemoveFriend&) { return CommandType::REMOVE_FRIEND; },
      [](const wire::GetFriends&) { return CommandType::GET_FRIENDS; },
      [](const wire::GetFriendRequests&) { return CommandType::GET_FRIEND_REQUESTS; },
      [](const wire::GetSentFriendRequests&) {
        return CommandType::GET_SENT_FRIEND_REQUESTS;
      },
      [](const wire::GetServerStats&) { return CommandType::GET_SERVER_STATS; },
      [](const wire::StartCall&) { return CommandType::START_CALL; },
      [](const wire::AcceptCall&) { return CommandType::ACCEPT_CALL; },
      [](const wire::RejectCall&) { return CommandType::REJECT_CALL; },
      [](const wire::EndCall&) { return CommandType::END_CALL; },
      [](const wire::Sdp&) { return CommandType::SDP; },
      [](const wire::IceCandidate&) { return CommandType::ICE_CANDIDATE; },
      [](const wire::GetTurnCredentials&) { return CommandType::GET_TURN_CREDENTIALS; },
      [](const wire::SetAvatar&) { return CommandType::SET_AVATAR; },
      [](const wire::GetOnlineUsers&) { return CommandType::GET_ONLINE_USERS; } },
    cmd);
}