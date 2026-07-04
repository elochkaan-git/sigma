#pragma once
#include "structures.h"
#include <QString>
#include <QUuid>

struct RegisterUserResponse
{
  QUuid client_id;
  OperationStatus status;
};

struct LoginUserResponse
{
  QUuid client_id;
  unsigned int user_id;
  OperationStatus status;
};

struct SendMessageResponse
{
  QUuid client_id;
  OperationStatus status;
};

struct NewMessageResponse
{
  QUuid client_id; // Получатель
  unsigned int sender_id;
  QString content;
};