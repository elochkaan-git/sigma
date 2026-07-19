#pragma once
#include "server_commands.h"
#include "server_responses.h"
#include "wire_command_types.h"

#include <variant>

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
                              GetSentFriendRequestsResponse,
                              GetServerStatsResponse,
                              StartCallResponse,
                              IncomingCallResponse,
                              AcceptCallResponse,
                              CallAcceptedResponse,
                              RejectCallResponse,
                              CallRejectedResponse,
                              EndCallResponse,
                              CallEndedResponse,
                              SdpResponse,
                              IceCandidateResponse,
                              GetTurnCredentialsResponse,
                              SetAvatarResponse,
                              GetOnlineUsersResponse>;
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
                             GetSentFriendRequests,
                             GetServerStats,
                             StartCall,
                             AcceptCall,
                             RejectCall,
                             EndCall,
                             Sdp,
                             IceCandidate,
                             GetTurnCredentials,
                             SetAvatar,
                             GetOnlineUsers>;

/**
 * @brief Возвращает тип команды
 *
 * @param cmd команда
 * @return CommandType
 */
CommandType
getTypeOfCommand(const Command& cmd);