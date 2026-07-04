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
   * @see Services
   */
  Dispatcher(Services services, OnlineUsersRegistry* registry);
  void dispatch(const Command& cmd,
                QObject* context,
                std::function<void(Response)> onResponseReady);

private:
  std::unique_ptr<QThreadPool> mThreadPool;
  Services mServices;
  OnlineUsersRegistry* mRegistry;
};