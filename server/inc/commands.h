#pragma once
#include <QUuid>
#include <QString>

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
