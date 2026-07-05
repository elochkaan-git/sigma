#pragma once
#include <QString>
#include <QUuid>

/**
 * @brief Нулевой объект для ошибок или остаточных путей в std::visit
 *
 * @see Dispatcher, NetworkManager
 */
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

/**
 * @brief Команда входа пользователя
 *
 */
struct LoginUser
{
  QUuid client_id;
  QString login;
  QString pwd;
};

/**
 * @brief Команда отправки сообщения
 *
 */
struct SendMessage
{
  QUuid client_id;
  unsigned int sender_id;
  unsigned int receiver_id;
  QString content;
};