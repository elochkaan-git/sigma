#pragma once
#include "connection_manager.h"
#include "structures.h"

#include <QString>

#include <optional>
#include <utility>
#include <vector>

/**
 * @brief Класс-репозиторий для работы с таблицей users
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
  OperationStatus registerUser(const QString& login, const QString& pwd_hash);
  /**
   * @brief Возвращает данные пользователя, если он зарегестрирован в базе
   * данных
   *
   * @param login логин пользователя
   * @return std::pair<OperationStatus, std::optional<User>> статус
   * операции и данные пользователя, если есть, иначе std::nullopt
   */
  std::pair<OperationStatus, std::optional<User>> getUserByLogin(
    const QString& login);
  /**
   * @brief Возвращает данные пользователя, если он зарегестрирован в базе
   * данных
   *
   * @param user_id id пользователя
   * @return std::pair<OperationStatus, std::optional<User>> статус
   * операции и данные пользователя, если есть, иначе std::nullopt
   */
  std::pair<OperationStatus, std::optional<User>> getUserByID(
    unsigned int user_id);
  /**
   * @brief Возвращает данные пользователей
   *
   * @param ids список id пользователей
   * @return std::pair<OperationStatus, std::optional<std::vector<User>>> статус
   * операции и данные пользователей, если есть, иначе std::nullopt
   */
  std::pair<OperationStatus, std::optional<std::vector<User>>> getUsersById(
    const std::vector<unsigned int>& ids);
  /**
   * @brief Возвращает число зарегистрированных пользователей
   *
   * @return std::pair<OperationStatus, unsigned int> статус операции и число
   * зарегистрированных пользователей
   */
  std::pair<OperationStatus, unsigned int> countUsers();
  std::pair<OperationStatus, std::optional<QString>> getUserPwdHash(
    const QString& login);
  /**
   * @brief Обновляет аватарку пользователя. Хранится как base64-строка
   *
   * @param user_id id пользователя
   * @param avatar_base64 изображение, закодированное в base64
   * @return OperationStatus статус выполнения операции
   */
  OperationStatus setAvatar(unsigned int user_id, const QString& avatar_base64);
  /**
   * @brief Обновляет время последней активности пользователя на текущее
   *
   * @param user_id id пользователя
   * @return OperationStatus статус выполнения операции
   */
  OperationStatus updateLastSeen(unsigned int user_id);

private:
  ConnectionManager* mConnManager;
};

/**
 * @brief Класс-репозиторий для работы с таблицей msgs_queue
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
                              const QString& content);
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
  OperationStatus deleteFromQueue(const std::vector<unsigned int>& msg_ids);

private:
  ConnectionManager* mConnManager;
};

/**
 * @brief Класс для работы с таблицей relations
 *
 */
class RelationRepository
{
public:
  /**
   * @brief Конструктор RelationRepository
   *
   * @param manager указатель на объект класса ConnectionManager
   */
  RelationRepository(ConnectionManager* manager);
  /**
   * @brief Отправляет заявку в друзья от пользователя user_id пользователю
   * friend_id
   *
   * @param user_id id отправителя заявки
   * @param friend_id id получателя заявки
   * @return OperationStatus статус операции
   */
  OperationStatus sendFriendRequest(unsigned int user_id,
                                    unsigned int friend_id);
  /**
   * @brief Принимает заявку в друзья от пользователя user_id пользователю
   * friend_id
   *
   * @param user_id id отправителя заявки
   * @param friend_id id получателя заявки
   * @return OperationStatus статус операции
   */
  OperationStatus acceptFriendRequest(unsigned int user_id,
                                      unsigned int friend_id);
  /**
   * @brief Отклоняет заявку в друзья от пользователя user_id пользователю
   * friend_id
   *
   * @param user_id id отправителя заявки
   * @param friend_id id получателя заявки
   * @return OperationStatus статус операции
   */
  OperationStatus rejectFriendRequest(unsigned int user_id,
                                      unsigned int friend_id);
  /**
   * @brief Удаляет дружбу между user_id и friend_id
   *
   * @param user_id id инициатора
   * @param friend_id id цели
   * @return OperationStatus статус операции
   */
  OperationStatus removeFriend(unsigned int user_id, unsigned int friend_id);
  /**
   * @brief Возвращает список id друзей
   *
   * @param user_id id пользователя
   * @return std::pair<OperationStatus, std::optional<std::vector<unsigned
   * int>>> статус операции и вектор id пользователей, если есть, иначе
   * std::nullopt
   */
  std::pair<OperationStatus, std::optional<std::vector<unsigned int>>>
  getFriendsID(unsigned int user_id);
  /**
   * @brief Возвращает список id людей, отправивших заявку в друзья
   *
   * @param user_id id пользователя
   * @return std::pair<OperationStatus, std::optional<std::vector<unsigned
   * int>>> статус операции и вектор id пользователей, если есть, иначе
   * std::nullopt
   */
  std::pair<OperationStatus, std::optional<std::vector<unsigned int>>>
  getFriendRequests(unsigned int user_id);
  /**
   * @brief Возвращает список id людей, которым была отправлена заявка в друзья
   *
   * @param user_id id пользователя
   * @return std::pair<OperationStatus, std::optional<std::vector<unsigned
   * int>>> статус операции и вектор id пользователей, если есть, иначе
   * std::nullopt
   */
  std::pair<OperationStatus, std::optional<std::vector<unsigned int>>>
  getSentFriendRequests(unsigned int user_id);
  /**
   * @brief Проверяет, являются ли user_id и friend_id друзьями
   *
   * @param user_id id инициатора
   * @param friend_id id цели
   * @return OperationStatus статус операции. OK, если пользователи являются
   * друзьями
   */
  OperationStatus areFriends(unsigned int user_id, unsigned int friend_id);

private:
  ConnectionManager* mConnManager;

private:
  /**
   * @brief Обрабатывает ошибки при выполнении QSqlQuery
   *
   * @param query очередь с подготовленными данными
   * @return OperationStatus статус операции
   */
  OperationStatus handleQueryErrors(QSqlQuery& query);
  std::pair<OperationStatus, std::optional<std::vector<unsigned int>>> getUsers(
    unsigned int user_id,
    QString user_status);
};