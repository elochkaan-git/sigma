#pragma once
#include <QString>
#include <QUuid>

struct NullCommand
{};

/**
 * @brief Команда регистрации пользователя
 *
 */
struct RegisterUser
{
  QUuid client_id;
  QString login;
  QString pwd;
};

struct LoginUser
{
  QUuid client_id;
  QString login;
  QString pwd;
};

struct SendMessage
{
  QUuid client_id;
  unsigned int sender_id;
  unsigned int receiver_id;
  QString content;
};