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

using Response = std::variant<Error,
                              RegisterUserResponse,
                              LoginUserResponse,
                              NewMessageResponse,
                              SendMessageResponse,
                              SendFriendRequestResponse,
                              AcceptFriendRequestResponse,
                              RejectFriendRequestResponse,
                              RemoveFriendResponse>;
using Command = std::variant<Error,
                             RegisterUser,
                             LoginUser,
                             SendMessage,
                             SendFriendRequest,
                             AcceptFriendRequest,
                             RejectFriendRequest,
                             RemoveFriend>;