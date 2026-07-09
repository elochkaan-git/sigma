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
  /**
   * @brief Возвращает user_id пользователя при успешном входе в аккаунт, иначе
   * 0 с соответствующим статусом выполнения
   *
   * @param login логин пользователя
   * @param passwd пароль
   * @return std::pair<OperationStatus, std::optional<unsigned int>> Статус
   * операции и user_id пользователя
   */
  std::pair<OperationStatus, std::optional<unsigned int>> loginUser(
    QString login,
    QString passwd);
  std::pair<OperationStatus, std::optional<User>> getUserByID(
    unsigned int user_id);

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
  MessageRepository* mMsgRepo;
};

class RelationService
{
public:
  RelationService(RelationRepository* rel_repo, UserRepository* u_repo);
  OperationStatus sendFriendRequest(unsigned int user_id,
                                    unsigned int friend_id);
  OperationStatus acceptFriendRequest(unsigned int user_id,
                                      unsigned int friend_id);
  OperationStatus rejectFriendRequest(unsigned int user_id,
                                      unsigned int friend_id);
  OperationStatus removeFriend(unsigned int user_id, unsigned int friend_id);
  std::pair<OperationStatus, std::optional<std::vector<User>>> getFriends(
    unsigned int user_id);
  std::pair<OperationStatus, std::optional<std::vector<User>>>
  getFriendRequests(unsigned int user_id);
  std::pair<OperationStatus, std::optional<std::vector<User>>>
  getSentFriendRequests(unsigned int user_id);
  OperationStatus areFriends(unsigned int user_id, unsigned int friend_id);

private:
  RelationRepository* mRelRepo;
  UserRepository* mUserRepo;
};

/**
 * @brief Структура, хранящая в себе указатели не сервисы
 *
 */
struct Services
{
  UserService* u_service;
  MessageService* msg_service;
  RelationService* rel_service;
};