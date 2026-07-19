#include "command_types.h"
#include "server_commands.h"
#include "wire_command_types.h"

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
      [](const GetServerStats&) { return CommandType::GET_SERVER_STATS; },
      [](const StartCall&) { return CommandType::START_CALL; },
      [](const AcceptCall&) { return CommandType::ACCEPT_CALL; },
      [](const RejectCall&) { return CommandType::REJECT_CALL; },
      [](const EndCall&) { return CommandType::END_CALL; },
      [](const Sdp&) { return CommandType::SDP; },
      [](const IceCandidate&) { return CommandType::ICE_CANDIDATE; },
      [](const GetTurnCredentials&) {
        return CommandType::GET_TURN_CREDENTIALS;
      } },
    cmd);
}