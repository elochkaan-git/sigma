#pragma once
#include "connection_manager.h"
#include "structures.h"
#include <QString>
#include <optional>
#include <utility>
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
  /**
   * @brief Возвращает данные пользователя, если он зарегестрирован в базе
   * данных
   *
   * @param login логин пользователя
   * @return std::pair<OperationStatus, std::optional<UserCredentials>> статус
   * операции и данные пользователя, если есть, иначе std::nullopt
   */
  std::pair<OperationStatus, std::optional<UserCredentials>> findUserByLogin(
    QString login);

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
   * @return OperationStatus статус выполнения операции
   * @see OperationStatus
   */
  OperationStatus saveToQueue(unsigned int sender_id,
                              unsigned int receiver_id,
                              QString content);
  /**
   * @brief Возвращает вектор писем, адресованных указанному пользователю
   *
   * @param receiver_id ID пользователя-получателя
   * @return std::pair<OperationStatus, std::optional<std::vector<Message>>>
   * Возвращает статус операции и вектор сообщений. Если сообщений в очереди
   * нет, то возвращается std::nullopt
   * @see OperationStatus, Message
   */
  std::pair<OperationStatus, std::optional<std::vector<Message>>>
  getQueuedMessages(unsigned int receiver_id);
  /**
   * @brief Удаляет из очереди письмо с указанным ID
   *
   * @param msg_id ID письма
   * @return OperationStatus статус выполнения операции
   * @see OperationStatus
   */
  OperationStatus deleteFromQueue(unsigned int msg_id);

private:
  ConnectionManager* mConnManager;
};