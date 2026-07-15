#include "dispatcher.h"

#include "command_types.h"
#include "commands.h"
#include "logging.h"
#include "responses.h"
#include "services.h"
#include "structures.h"
#include "task.h"

#include <QObject>
#include <QThreadPool>

#include <functional>
#include <memory>
#include <optional>
#include <qlogging.h>
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
  qInfo(appDispatcher) << "Got new command: " +
                            commandTypeToString(getTypeOfCommand(cmd));
  std::function<std::vector<Response>()> job = std::visit(
    overloaded{
      [this](
        const RegisterUser& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleRegisterUser(cmd);
        };
      },
      [this](const LoginUser& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleLoginUser(cmd);
        };
      },
      [this](const SendMessage& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleMessage(cmd);
        };
      },
      [this](const SendFriendRequest& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleSendFriendRequest(cmd);
        };
      },
      [this](const AcceptFriendRequest& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleAcceptFriendRequest(cmd);
        };
      },
      [this](const RejectFriendRequest& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleRejectFriendRequest(cmd);
        };
      },
      [this](
        const RemoveFriend& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleRemoveFriend(cmd);
        };
      },
      [this](const GetFriends& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleGetFriends(cmd);
        };
      },
      [this](const GetFriendRequests& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleGetFriendRequests(cmd);
        };
      },
      [this](const GetSentFriendRequests& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleGetSentFriendRequests(cmd);
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

std::vector<Response>
Dispatcher::handleRegisterUser(const RegisterUser& cmd)
{
  OperationStatus status =
    this->mServices.u_service->registerUser(cmd.login, cmd.pwd);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command RegisterUser return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("User %1 registered").arg(cmd.client_id.toString());
  }
  return std::vector<Response>{ RegisterUserResponse{ cmd.client_id, status } };
}

std::vector<Response>
Dispatcher::handleLoginUser(const LoginUser& cmd)
{
  auto [status, user_id] = mServices.u_service->loginUser(cmd.login, cmd.pwd);
  if (user_id.has_value()) {
    std::vector<Response> responses{ LoginUserResponse{
      cmd.client_id, user_id.value(), OperationStatus::OK } };
    auto [status, msgs] =
      this->mServices.msg_service->getQueuedMessages(user_id.value());
    if (msgs.has_value() && status == OperationStatus::OK) {
      for (const auto& m : msgs.value()) {
        responses.push_back(
          NewMessageResponse{ cmd.client_id, m.sender_id, m.content });
        status = this->mServices.msg_service->deleteFromQueue(m.msg_id);
        if (status != OperationStatus::OK) {
          qWarning(appDispatcher)
            << QString("Command LoginUser return status %1").arg((int)status);
          break;
        }
      }
      if (status != OperationStatus::OK) {
        qWarning(appDispatcher) << "Not all messages from queue was processed";
      } else {
        qInfo(appDispatcher) << "All messages from queue was processed";
      }
    }
    return responses;
  } else {
    qWarning(appDispatcher)
      << QString("Command LoginUser return status %1").arg((int)status);
    return std::vector<Response>{ LoginUserResponse{
      cmd.client_id, 0, OperationStatus::InvalidCredentials } };
  }
}

std::vector<Response>
Dispatcher::handleMessage(const SendMessage& cmd)
{
  OperationStatus status =
    this->mServices.rel_service->areFriends(cmd.sender_id, cmd.receiver_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command SendMessage return status %1").arg((int)status);
    return std::vector<Response>{ SendMessageResponse{ cmd.client_id,
                                                       status } };
  }
  std::optional<QUuid> receiver_id =
    this->mRegistry->getClientId(cmd.receiver_id);
  if (receiver_id.has_value()) {
    qInfo(appDispatcher) << QString("Sending message from %1 to %2")
                              .arg(cmd.sender_id)
                              .arg(cmd.receiver_id);
    return std::vector<Response>{
      SendMessageResponse{ cmd.client_id, OperationStatus::OK },
      NewMessageResponse{ receiver_id.value(), cmd.sender_id, cmd.content }
    };
  } else {
    qInfo(appDispatcher)
      << QString("Saving message from %1 in queue").arg(cmd.sender_id);
    OperationStatus status = mServices.msg_service->saveToQueue(
      cmd.sender_id, cmd.receiver_id, cmd.content);
    return std::vector<Response>{ SendMessageResponse{ cmd.client_id,
                                                       status } };
  }
}

std::vector<Response>
Dispatcher::handleSendFriendRequest(const SendFriendRequest& cmd)
{

  OperationStatus status =
    this->mServices.rel_service->sendFriendRequest(cmd.user_id, cmd.friend_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command SendFriendRequest return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher) << QString("Sending friend request from %1 to %2")
                              .arg(cmd.user_id)
                              .arg(cmd.friend_id);
  }
  return std::vector<Response>{ SendFriendRequestResponse{ cmd.client_id,
                                                           status } };
}

std::vector<Response>
Dispatcher::handleAcceptFriendRequest(const AcceptFriendRequest& cmd)
{
  OperationStatus status = this->mServices.rel_service->acceptFriendRequest(
    cmd.user_id, cmd.friend_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command AcceptFriendRequest return status %1")
           .arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("%1 and %2 friends now").arg(cmd.user_id).arg(cmd.friend_id);
  }
  return std::vector<Response>{ AcceptFriendRequestResponse{ cmd.client_id,
                                                             status } };
}

std::vector<Response>
Dispatcher::handleRejectFriendRequest(const RejectFriendRequest& cmd)
{
  OperationStatus status = this->mServices.rel_service->rejectFriendRequest(
    cmd.user_id, cmd.friend_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command RejectFriendRequest return status %1")
           .arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("%1 reject %2").arg(cmd.user_id).arg(cmd.friend_id);
  }
  return std::vector<Response>{ RejectFriendRequestResponse{ cmd.client_id,
                                                             status } };
}

std::vector<Response>
Dispatcher::handleRemoveFriend(const RemoveFriend& cmd)
{
  OperationStatus status =
    this->mServices.rel_service->removeFriend(cmd.user_id, cmd.friend_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command RemoveFriend return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher) << QString("%1 not %2 friends anymore")
                              .arg(cmd.user_id)
                              .arg(cmd.friend_id);
  }
  return std::vector<Response>{ RemoveFriendResponse{ cmd.client_id, status } };
}

std::vector<Response>
Dispatcher::handleGetFriends(const GetFriends& cmd)
{
  auto [status, friends] = this->mServices.rel_service->getFriends(cmd.user_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command GetFriends return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("Sending friends list to %1").arg(cmd.user_id);
  }
  return std::vector<Response>{ GetFriendsResponse{
    cmd.client_id, status, friends } };
}

std::vector<Response>
Dispatcher::handleGetFriendRequests(const GetFriendRequests& cmd)
{
  auto [status, requests] =
    this->mServices.rel_service->getFriendRequests(cmd.user_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command GetFriendRequests return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("Sending friend requests list to %1").arg(cmd.user_id);
  }
  return std::vector<Response>{ GetFriendRequestsResponse{
    cmd.client_id, status, requests } };
}

std::vector<Response>
Dispatcher::handleGetSentFriendRequests(const GetSentFriendRequests& cmd)
{
  auto [status, sentRequests] =
    this->mServices.rel_service->getSentFriendRequests(cmd.user_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command GetFriendRequests return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("Sending sent friend requests list to %1").arg(cmd.user_id);
  }
  return std::vector<Response>{ GetSentFriendRequestsResponse{
    cmd.client_id, status, sentRequests } };
}
