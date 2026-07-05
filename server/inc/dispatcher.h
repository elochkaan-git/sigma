#pragma once
#include "command_types.h"
#include "registry.h"
#include "services.h"
#include <QThreadPool>
#include <functional>
#include <memory>

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
};