#pragma once
#include "statuses.h"
#include <QUuid>

struct RegisterUserResponse
{
  QUuid client_id;
  OperationStatus status;
};