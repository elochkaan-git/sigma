#include "dispatcher.h"
#include "command_types.h"
#include "services.h"
#include "statuses.h"
#include "task.h"
#include <QObject>
#include <QThreadPool>
#include <memory>
#include <vector>

Dispatcher::Dispatcher(Services services)
  : mServices(services)
{
  mThreadPool = std::make_unique<QThreadPool>();
}

void
Dispatcher::dispatch(const Command& cmd,
                     QObject* context,
                     std::function<void(Response)> onResponseReady)
{
  std::visit(
    overloaded{ [this, context, onResponseReady](const RegisterUser& cmd) {
      auto job = [this, cmd]() -> std::vector<Response> {
        OperationStatus status =
          this->mServices.u_service->registerUser(cmd.login, cmd.pwd);
        return std::vector<Response>{ RegisterUserResponse{ cmd.client_id,
                                                            status } };
      };
      Task* task = new Task(std::move(
        job)); // Владение передается QThreadPool, утечки памяти не будет
      QObject::connect(task, &Task::responseReady, context, onResponseReady);
      this->mThreadPool->start(task);
    } },
    cmd);
}