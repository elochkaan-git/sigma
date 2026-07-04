#pragma once
#include "commands.h"
#include "responses.h"
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

using Response = std::variant<RegisterUserResponse>;
using Command = std::variant<RegisterUser>;