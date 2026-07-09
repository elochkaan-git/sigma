#include "dispatcher.h"
#include "command_types.h"
#include "commands.h"
#include "responses.h"
#include "services.h"
#include "structures.h"
#include "task.h"
#include <QObject>
#include <QThreadPool>
#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

Dispatcher::Dispatcher(Services services, OnlineUsersRegistry* registry)
  : mServices(services)
  , mRegistry(registry)
{
  mThreadPool = std::make_unique<QThreadPool>();
}

void
Dispatcher::dispatch(const Command& cmd,
                     QObject* context,
                     std::function<void(Response)> onResponseReady)
{
  std::visit(
    overloaded{
      [this, context, onResponseReady](const RegisterUser& cmd) {
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
      },
      [this, context, onResponseReady](const LoginUser& cmd) {
        auto job = [this, cmd]() -> std::vector<Response> {
          auto [status, user_id] =
            mServices.u_service->loginUser(cmd.login, cmd.pwd);
          if (user_id.has_value()) {
            std::vector<Response> responses{ LoginUserResponse{
              cmd.client_id, user_id.value(), OperationStatus::OK } };
            responses.reserve(100);
            auto [status, msgs] =
              this->mServices.msg_service->getQueuedMessages(user_id.value());
            for (const auto& m : msgs.value()) {
              responses.push_back(
                NewMessageResponse{ cmd.client_id, m.sender_id, m.content });
              this->mServices.msg_service->deleteFromQueue(m.msg_id);
            }
            return responses;
          } else {
            return std::vector<Response>{ LoginUserResponse{
              cmd.client_id, 0, OperationStatus::InvalidCredentials } };
          }
        };
        Task* task = new Task(std::move(job));
        QObject::connect(task, &Task::responseReady, context, onResponseReady);
        this->mThreadPool->start(task);
      },
      [this, context, onResponseReady](const SendMessage& cmd) {
        auto job = [this, cmd]() -> std::vector<Response> {
          const auto [status, ids] =
            mServices.rel_service->getFriends(cmd.receiver_id);
          if (status != OperationStatus::OK && !ids.has_value()) {
            return std::vector<Response>{ SendMessageResponse{ cmd.client_id,
                                                               status } };
          }
          if (std::find_if(ids.value().begin(),
                           ids.value().end(),
                           [cmd](const User& user) {
                             return user.user_id == cmd.sender_id;
                           }) == ids.value().end()) {
            return std::vector<Response>{ SendMessageResponse{
              cmd.client_id, OperationStatus::UserNotInFriends } };
          }
          std::optional<QUuid> receiver_id =
            this->mRegistry->getClientId(cmd.receiver_id);
          if (receiver_id.has_value()) {
            return std::vector<Response>{
              SendMessageResponse{ cmd.client_id, OperationStatus::OK },
              NewMessageResponse{
                receiver_id.value(), cmd.sender_id, cmd.content }
            };
          } else {
            mServices.msg_service->saveToQueue(
              cmd.sender_id, cmd.receiver_id, cmd.content);
            return std::vector<Response>{ SendMessageResponse{
              cmd.client_id, OperationStatus::OK } };
          }
        };
        Task* task = new Task(std::move(job));
        QObject::connect(task, &Task::responseReady, context, onResponseReady);
        this->mThreadPool->start(task);
      }, //TODO: сделать обработку команд, связанных с заявками в друзья
      [](const auto&) {} },
    cmd);
}