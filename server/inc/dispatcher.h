#pragma once
#include <QThreadPool>
#include <functional>
#include <memory>
#include "command_types.h"
#include "services.h"

class Dispatcher
{
public:
  Dispatcher(UserService* user_service);
  void dispatch(const Command& cmd, QObject* context, std::function<void(Response)> onResponseReady);

private:
  std::unique_ptr<QThreadPool> mThreadPool;
  UserService* mUserService;
};