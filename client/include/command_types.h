#pragma once
#include "commands.h"
#include "responses.h"
#include "structures.h"
#include "wire_command_types.h"

#include <variant>

/**
 * @brief сокращение для варианта ответов
 */
using Response = std::variant<wire::Error,
                              wire::RegisterUserResponse,
                              wire::LoginUserResponse,
                              wire::NewMessageResponse,
                              wire::SendMessageResponse,
                              wire::SendFriendRequestResponse,
                              wire::AcceptFriendRequestResponse,
                              wire::RejectFriendRequestResponse,
                              wire::RemoveFriendResponse,
                              wire::GetFriendsResponse,
                              wire::GetFriendRequestsResponse,
                              wire::GetSentFriendRequestsResponse,
                              wire::GetServerStatsResponse,
                              wire::StartCallResponse,
                              wire::IncomingCallResponse,
                              wire::AcceptCallResponse,
                              wire::CallAcceptedResponse,
                              wire::RejectCallResponse,
                              wire::CallRejectedResponse,
                              wire::EndCallResponse,
                              wire::CallEndedResponse,
                              wire::SdpResponse,
                              wire::IceCandidateResponse,
                              wire::GetTurnCredentialsResponse,
                              wire::SetAvatarResponse,
                              wire::GetOnlineUsersResponse>;
/**
 * @brief сокращение для варианта команд
 */
using Command = std::variant<wire::Error,
                             wire::RegisterUser,
                             wire::LoginUser,
                             wire::SendMessage,
                             wire::SendFriendRequest,
                             wire::AcceptFriendRequest,
                             wire::RejectFriendRequest,
                             wire::RemoveFriend,
                             wire::GetFriends,
                             wire::GetFriendRequests,
                             wire::GetSentFriendRequests,
                             wire::GetServerStats,
                             wire::StartCall,
                             wire::AcceptCall,
                             wire::RejectCall,
                             wire::EndCall,
                             wire::Sdp,
                             wire::IceCandidate,
                             wire::GetTurnCredentials,
                             wire::SetAvatar,
                             wire::GetOnlineUsers>;

/**
 * @brief Возвращает тип команды
 *
 * @param cmd команда
 * @return CommandType
 */
CommandType
getTypeOfCommand(const Command& cmd);