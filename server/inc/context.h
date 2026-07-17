#pragma once
#include "structures.h"

#include <QUuid>

struct ClientCtx
{
  QUuid client_id;
};

struct SessionCtx : public ClientCtx
{
  unsigned int user_id;
};

struct Error
  : public wire::Error
  , public ClientCtx
{};