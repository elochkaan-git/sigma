#include "dispatcher.h"
#include "command_types.h"
#include "services.h"
#include "statuses.h"
#include "task.h"
#include <memory>
#include <QThreadPool>
#include <QObject>

Dispatcher::Dispatcher(UserService* user_service)
  : mUserService(user_service)
{
  mThreadPool = std::make_unique<QThreadPool>();
}

void
Dispatcher::dispatch(const Command& cmd, QObject* context, std::function<void(Response)> onResponseReady)
{
  std::visit(overloaded{
    [this, context, onResponseReady](const RegisterUser& cmd) {
      auto job = [this, cmd]() -> Response {
        OperationStatus status = this->mUserService->registerUser(cmd.login, cmd.pwd);
        return RegisterUserResponse{cmd.client_id, status};
      };
      Task* task = new Task(std::move(job)); // Владение передается QThreadPool, утечки памяти не будет
      QObject::connect(task, &Task::responseReady, context, onResponseReady);
      this->mThreadPool->start(task);
    }
  }, cmd);
}