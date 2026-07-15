#pragma once
#include "command_types.h"
#include "commands.h"
#include "registry.h"
#include "services.h"

#include <QThreadPool>

#include <functional>
#include <memory>
#include <vector>

/**
 * @brief Класс-диспетчер, распределяющий задачи по потокам
 *
 */
class Dispatcher
{
public:
  /**
   * @brief Конструктор класса Dispatcher
   *
   * @param services структура с указателями на сервисы
   * @param registry реестр онлайн пользователей
   * @see Services
   * @see OnlineUsersRegistry
   */
  Dispatcher(Services services, OnlineUsersRegistry* registry);
  /**
   * @brief Метод распределения команд по разным потокам
   *
   * @param cmd Команда
   * @param context Контекст. Необходим для выполнения Task в том же потоке, что
   * и передаваемый объект. Необходимо, что испускаемые сигналы были в том же
   * потоке, что и NetworkManager
   * @param onResponseReady
   * @see commands.h
   * @see Task
   * @see NetworkManager
   */
  void dispatch(const Command& cmd,
                QObject* context,
                std::function<void(Response)> onResponseReady);

private:
  std::unique_ptr<QThreadPool> mThreadPool;
  Services mServices;
  OnlineUsersRegistry* mRegistry;

private:
  /**
   * @brief Метод обработки регистрации пользователя
   *
   * @param cmd команда RegisterUser
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleRegisterUser(const RegisterUser& cmd);
  /**
   * @brief Метод обработки входа пользователя
   *
   * @param cmd команда LoginUser
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleLoginUser(const LoginUser& cmd);
  /**
   * @brief Метод обработки присланного сообщения
   *
   * @param cmd команда SendMessage
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleMessage(const SendMessage& cmd);
  /**
   * @brief Метод обработки отправленной заявки в друзья
   *
   * @param cmd команда SendFriendRequest
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleSendFriendRequest(const SendFriendRequest& cmd);
  /**
   * @brief Метод обработки принятия заявки в друзья
   *
   * @param cmd команда AcceptFriendRequest
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleAcceptFriendRequest(
    const AcceptFriendRequest& cmd);
  /**
   * @brief Метод обработки отклонения заявки в друзья
   *
   * @param cmd команда RejectFriendRequest
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleRejectFriendRequest(
    const RejectFriendRequest& cmd);
  /**
   * @brief Метод обработки удаления пользователя из друзей
   *
   * @param cmd команда RemoveFriend
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleRemoveFriend(const RemoveFriend& cmd);
  /**
   * @brief Метод обработки получения списка друзей
   *
   * @param cmd команда GetFriends
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleGetFriends(const GetFriends& cmd);
  /**
   * @brief Метод обработки получения списка заявок в друзья
   *
   * @param cmd команда GetFriendRequests
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleGetFriendRequests(const GetFriendRequests& cmd);
  /**
   * @brief Метод обработки получения списка отправленных заявок в друзья
   *
   * @param cmd команда GetSentFriendRequests
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleGetSentFriendRequests(
    const GetSentFriendRequests& cmd);
  /**
   * @brief Метод обработки получения статистики сервера
   * 
   * @param cmd команда GetServerStats
   * @return std::vector<Response> список ответов
   */
  std::vector<Response> handleGetServerStats(const GetServerStats& cmd);
};