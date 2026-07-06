#pragma once
#include "connection_manager.h"
#include "structures.h"
#include <QString>
#include <optional>
#include <vector>

/**
 * @brief Класс-репозиторий для работы с таблицей Users
 *
 */
class UserRepository
{
public:
  /**
   * @brief Конструктор UserRepository
   *
   * @param manager указатель на объект класса ConnectionManager
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

/**
 * @brief Класс-репозиторий для работы с таблицей Msgs_queue
 *
 */
class MessageRepository
{
public:
  /**
   * @brief Конструктор MessageRepository
   *
   * @param manager указатель на объект класса ConnectionManager
   * @see ConnectionManager
   */
  MessageRepository(ConnectionManager* manager);
  /**
   * @brief Сохраняет письмо в очереь
   *
   * @param sender_id ID пользователя-отправителя
   * @param receiver_id ID пользователя-получателя
   * @param content содержимое письма
   */
  void saveToQueue(unsigned int sender_id,
                   unsigned int receiver_id,
                   QString content);
  /**
   * @brief Возвращает вектор писем, адресованных указанному пользователю
   *
   * @param receiver_id ID пользователя-получателя
   * @return std::vector<Message> Вектор сообщений. Может быть пустым
   */
  std::vector<Message> getQueuedMessages(unsigned int receiver_id);
  /**
   * @brief Удаляет из очереди письмо с указанным ID
   *
   * @param msg_id ID письма
   */
  void deleteFromQueue(unsigned int msg_id);

private:
  ConnectionManager* mConnManager;
};