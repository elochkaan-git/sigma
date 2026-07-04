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
   * @param repo указатель на UserRepository
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

class MessageService
{
public:
  MessageService(MessageRepository* repo);
  void saveToQueue(unsigned int sender_id,
                   unsigned int receiver_id,
                   QString content);
  std::vector<Message> getQueuedMessages(unsigned int user_id);
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