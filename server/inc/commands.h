#pragma once
#include <QString>
#include <QUuid>

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
