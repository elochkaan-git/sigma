#pragma once
#include "repositories.h"
#include "structures.h"
#include <QObject>
#include <QString>
#include <optional>

/**
 * @brief Класс-сервис для работы с репозиторием UserRepository в базе данных.
 *
 * @see UserRepository
 */
class UserService
{
public:
  /**
   * @brief Конструктор сервиса.
   *
   * @param repo указатель на объект класса UserRepository
   */
  UserService(UserRepository* u_repo);
  /**
   * @brief Метод для регистрации пользователей. Принимает нехэшированный пароль
   * и логин, возвращает статус выполнения операции
   *
   * @param login логин пользователя
   * @param passwd нехэшированный пароль
   * @return OperationStatus статус операции
   *
   * @see OperationStatus
   */
  OperationStatus registerUser(QString login, QString passwd);
  std::optional<unsigned int> loginUser(QString login, QString passwd);

private:
  UserRepository* mUserRepo;
};

/**
 * @brief Класс-сервис для работы с письмами
 *
 */
class MessageService
{
public:
  /**
   * @brief Конструктор MessageService
   *
   * @param repo указатель на объект класса MessageRepository
   */
  MessageService(MessageRepository* repo);
  /**
   * @brief Сохраняет письмо в очередь
   *
   * @param sender_id ID пользователя-отправителя
   * @param receiver_id ID пользователя-получателя
   * @param content контент письма
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
  std::vector<Message> getQueuedMessages(unsigned int user_id);
  /**
   * @brief Удаляет из очереди письмо с указанным ID
   *
   * @param msg_id ID письма
   */
  void deleteFromQueue(unsigned int msg_id);

private:
  MessageRepository* mMsgRepo;
};

/**
 * @brief Структура, хранящая в себе указатели не сервисы
 *
 */
struct Services
{
  UserService* u_service;
  MessageService* msg_service;
};