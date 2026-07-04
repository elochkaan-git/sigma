#pragma once
#include "connection_manager.h"
#include "structures.h"
#include <QString>
#include <optional>
#include <vector>

/**
 * @brief Класс для непосредственной работы с таблицами в базе данных
 *
 */
class UserRepository
{
public:
  /**
   * @brief Конструктор UserRepository
   *
   * @param manager указатель на ConnectionManager
   * @see ConnectionManager
   */
  UserRepository(ConnectionManager* manager);
  /**
   * @brief Метод регистрации пользователя. Принимает логин и хэшированный
   * пароль. В случае успеха, возвращает ОК, при существовании пользователя
   * возвращает UserExist.
   *
   * @param login логин пользователя
   * @param pwd_hash хэш пароля
   * @return OperationStatus статус выполнения операции
   * @see OperationStatus
   */
  OperationStatus registerUser(QString login, QString pwd_hash);
  std::optional<UserCredentials> findUserByLogin(QString login);

private:
  ConnectionManager* mConnManager;
};

class MessageRepository
{
public:
  MessageRepository(ConnectionManager* manager);
  void saveToQueue(unsigned int sender_id,
                   unsigned int receiver_id,
                   QString content);
  std::vector<Message> getQueuedMessages(unsigned int user_id);
  void deleteFromQueue(unsigned int msg_id);

private:
  ConnectionManager* mConnManager;
};