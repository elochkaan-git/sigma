#pragma once
#include <QUuid>
#include "statuses.h"

struct RegisterUserResponse
{
  QUuid client_id;
  OperationStatus status;
};