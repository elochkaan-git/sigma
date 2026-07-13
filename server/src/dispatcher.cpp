#include "dispatcher.h"

#include "command_types.h"
#include "commands.h"
#include "responses.h"
#include "services.h"
#include "structures.h"
#include "task.h"

#include <QObject>
#include <QThreadPool>

#include <functional>
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
  std::function<std::vector<Response>()> job = std::visit(
    overloaded{
      [this](
        const RegisterUser& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          OperationStatus status =
            this->mServices.u_service->registerUser(cmd.login, cmd.pwd);
          return std::vector<Response>{ RegisterUserResponse{ cmd.client_id,
                                                              status } };
        };
      },
      [this](const LoginUser& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          auto [status, user_id] =
            mServices.u_service->loginUser(cmd.login, cmd.pwd);
          if (user_id.has_value()) {
            std::vector<Response> responses{ LoginUserResponse{
              cmd.client_id, user_id.value(), OperationStatus::OK } };
            auto [status, msgs] =
              this->mServices.msg_service->getQueuedMessages(user_id.value());
            if (msgs.has_value() && status == OperationStatus::OK) {
              for (const auto& m : msgs.value()) {
                responses.push_back(
                  NewMessageResponse{ cmd.client_id, m.sender_id, m.content });
                this->mServices.msg_service->deleteFromQueue(m.msg_id);
              }
            }
            return responses;
          } else {
            return std::vector<Response>{ LoginUserResponse{
              cmd.client_id, 0, OperationStatus::InvalidCredentials } };
          }
        };
      },
      [this](const SendMessage& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          OperationStatus status = this->mServices.rel_service->areFriends(
            cmd.sender_id, cmd.receiver_id);
          if (status != OperationStatus::OK) {
            return std::vector<Response>{ SendMessageResponse{ cmd.client_id,
                                                               status } };
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
            OperationStatus status = mServices.msg_service->saveToQueue(
              cmd.sender_id, cmd.receiver_id, cmd.content);
            return std::vector<Response>{ SendMessageResponse{ cmd.client_id,
                                                               status } };
          }
        };
      },
      [this](const SendFriendRequest& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          OperationStatus status =
            this->mServices.rel_service->sendFriendRequest(cmd.user_id,
                                                           cmd.friend_id);
          return std::vector<Response>{ SendFriendRequestResponse{
            cmd.client_id, status } };
        };
      },
      [this](const AcceptFriendRequest& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          OperationStatus status =
            this->mServices.rel_service->acceptFriendRequest(cmd.user_id,
                                                             cmd.friend_id);
          return std::vector<Response>{ AcceptFriendRequestResponse{
            cmd.client_id, status } };
        };
      },
      [this](const RejectFriendRequest& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          OperationStatus status =
            this->mServices.rel_service->rejectFriendRequest(cmd.user_id,
                                                             cmd.friend_id);
          return std::vector<Response>{ RejectFriendRequestResponse{
            cmd.client_id, status } };
        };
      },
      [this](
        const RemoveFriend& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          OperationStatus status = this->mServices.rel_service->removeFriend(
            cmd.user_id, cmd.friend_id);
          return std::vector<Response>{ SendFriendRequestResponse{
            cmd.client_id, status } };
        };
      },
      [](const Error& cmd) -> std::function<std::vector<Response>()> {
        auto job = [cmd]() -> std::vector<Response> {
          return std::vector<Response>{ cmd };
        };
        return job;
      } },
    cmd);
  Task* task = new Task(std::move(job));
  QObject::connect(task, &Task::responseReady, context, onResponseReady);
  this->mThreadPool->start(task);
}