#include "dispatcher.h"

#include "call_registry.h"
#include "command_types.h"
#include "logging.h"
#include "server_commands.h"
#include "server_responses.h"
#include "services.h"
#include "structures.h"
#include "task.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QObject>
#include <QSettings>
#include <QThreadPool>

#include <functional>
#include <memory>
#include <optional>
#include <qlogging.h>
#include <quuid.h>
#include <type_traits>
#include <vector>

namespace {

/**
 * @brief Type-trait, определяющий, есть ли у команды публичное поле user_id
 * (т.е. является ли она SessionCtx-командой, пришедшей от авторизованного
 * пользователя). Используется для централизованного обновления времени
 * последней активности пользователя в Dispatcher::dispatch
 */
template<typename T, typename = void>
struct has_user_id : std::false_type
{};

template<typename T>
struct has_user_id<T, std::void_t<decltype(std::declval<T>().user_id)>>
  : std::true_type
{};

} // namespace

Dispatcher::Dispatcher(Services services,
                       OnlineUsersRegistry* registry,
                       CallRegistry* call_registry)
  : mServices(services)
  , mRegistry(registry)
  , mCallRegistry(call_registry)
{
  mThreadPool = std::make_unique<QThreadPool>();
  mThreadPool->setMaxThreadCount(8);
  QSettings settings("config.ini", QSettings::IniFormat);
  settings.beginGroup("turn");
  mTurnSecret = settings.value("secret", "").toString();
  settings.endGroup();
  if (mTurnSecret.isEmpty()) {
    qCritical(appDispatcher)
      << "TURN secret not set in config! TURN credentials will not work.";
  }
}

void
Dispatcher::dispatch(const Command& cmd,
                     QObject* context,
                     std::function<void(Response)> onResponseReady)
{
  qInfo(appDispatcher) << "Got new command: " +
                            commandTypeToString(getTypeOfCommand(cmd));

  // Централизованное обновление времени последней активности: срабатывает
  // для любой команды, у которой есть публичное поле user_id (то есть для
  // всех SessionCtx-команд от уже авторизованных пользователей). LoginUser
  // обрабатывается отдельно в handleLoginUser, так как на момент прихода
  // команды user_id еще не известен.
  std::visit(
    [this](auto&& c) {
      using T = std::decay_t<decltype(c)>;
      if constexpr (has_user_id<T>::value) {
        this->mServices.u_service->updateLastSeen(c.user_id);
      }
    },
    cmd);

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
      [this](
        const GetServerStats& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleGetServerStats(cmd);
        };
      },
      [this](const StartCall& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleStartCall(cmd);
        };
      },
      [this](const AcceptCall& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleAcceptCall(cmd);
        };
      },
      [this](const RejectCall& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleRejectCall(cmd);
        };
      },
      [this](const EndCall& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleEndCall(cmd);
        };
      },
      [this](const Sdp& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleSdp(cmd);
        };
      },
      [this](
        const IceCandidate& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleIceCandidate(cmd);
        };
      },
      [this](const GetTurnCredentials& cmd)
        -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleGetTurnCredentials(cmd);
        };
      },
      [this](const SetAvatar& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleSetAvatar(cmd);
        };
      },
      [this](
        const GetOnlineUsers& cmd) -> std::function<std::vector<Response>()> {
        return [this, cmd]() -> std::vector<Response> {
          return this->handleGetOnlineUsers(cmd);
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
  RegisterUserResponse r;
  r.client_id = cmd.client_id;
  r.status = status;
  return std::vector<Response>{ r };
}

std::vector<Response>
Dispatcher::handleLoginUser(const LoginUser& cmd)
{
  auto [status, user_id] = mServices.u_service->loginUser(cmd.login, cmd.pwd);
  LoginUserResponse lur;
  lur.client_id = cmd.client_id;

  if (user_id.has_value() && status == OperationStatus::OK) {
    lur.user_id = user_id.value();
    lur.status = OperationStatus::OK;

    this->mServices.u_service->updateLastSeen(user_id.value());

    auto [status, msgs] =
      this->mServices.msg_service->getQueuedMessages(user_id.value());

    std::vector<Response> responses{ lur };

    if (msgs.has_value() && status == OperationStatus::OK) {
      const auto& messages = msgs.value();
      if (!messages.empty()) {
        std::vector<unsigned int> ids;
        ids.reserve(messages.size());

        for (const auto& m : messages) {
          ids.push_back(m.msg_id);
        }

        OperationStatus delStatus =
          this->mServices.msg_service->deleteFromQueue(ids);
        if (delStatus == OperationStatus::OK) {
          for (const auto& m : messages) {
            NewMessageResponse nmr;
            nmr.client_id = cmd.client_id;
            nmr.sender_id = m.sender_id;
            nmr.content = m.content;
            responses.push_back(nmr);
          }
          qInfo(appDispatcher)
            << "Queued messages delivered to user" << user_id.value();
        } else {
          qWarning(appDispatcher)
            << "Failed to delete queued messages for user" << user_id.value()
            << "will retry on next login";
        }
      } else {
        qInfo(appDispatcher)
          << "No queued messages for user" << user_id.value();
      }
    } else {
      qWarning(appDispatcher)
        << "Failed to get queued messages or no messages" << (int)status;
    }

    return responses;
  } else {
    lur.user_id = 0;
    lur.status = status;
    qWarning(appDispatcher)
      << "Login failed for" << cmd.login << "status" << (int)status;
    return std::vector<Response>{ lur };
  }
}

std::vector<Response>
Dispatcher::handleMessage(const SendMessage& cmd)
{
  OperationStatus status =
    this->mServices.rel_service->areFriends(cmd.user_id, cmd.receiver_id);
  SendMessageResponse smr;
  smr.client_id = cmd.client_id;
  smr.status = status;
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command SendMessage return status %1").arg((int)status);
    return std::vector<Response>{ smr };
  }
  std::optional<QUuid> receiver_id =
    this->mRegistry->getClientId(cmd.receiver_id);
  if (receiver_id.has_value()) {
    qInfo(appDispatcher) << QString("Sending message from %1 to %2")
                              .arg(cmd.user_id)
                              .arg(cmd.receiver_id);
    NewMessageResponse nmr;
    nmr.client_id = receiver_id.value();
    nmr.sender_id = cmd.user_id;
    nmr.content = cmd.content;
    return std::vector<Response>{ smr, nmr };
  } else {
    qInfo(appDispatcher)
      << QString("Saving message from %1 in queue").arg(cmd.user_id);
    status = mServices.msg_service->saveToQueue(
      cmd.user_id, cmd.receiver_id, cmd.content);
    if (status != OperationStatus::OK) {
      qWarning(appDispatcher)
        << QString("Message from %1 wasn't saved to queue").arg(cmd.user_id);
    } else {
      qWarning(appDispatcher)
        << QString("Messagese from %1 saved!").arg(cmd.user_id);
    }
    smr.status = status;
    return std::vector<Response>{ smr };
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
  SendFriendRequestResponse sfrr;
  sfrr.client_id = cmd.client_id;
  sfrr.status = status;
  return std::vector<Response>{ sfrr };
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
  AcceptFriendRequestResponse afrr;
  afrr.client_id = cmd.client_id;
  afrr.status = status;
  return std::vector<Response>{ afrr };
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
  RejectFriendRequestResponse rfrr;
  rfrr.client_id = cmd.client_id;
  rfrr.status = status;
  return std::vector<Response>{ rfrr };
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
  RemoveFriendResponse rfr;
  rfr.client_id = cmd.client_id;
  rfr.status = status;
  return std::vector<Response>{ rfr };
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
  GetFriendsResponse gfr;
  gfr.client_id = cmd.client_id;
  gfr.status = status;
  gfr.friends = friends;
  return std::vector<Response>{ gfr };
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
  GetFriendRequestsResponse gfrr;
  gfrr.client_id = cmd.client_id;
  gfrr.status = status;
  gfrr.requests = requests;
  return std::vector<Response>{ gfrr };
}

std::vector<Response>
Dispatcher::handleGetSentFriendRequests(const GetSentFriendRequests& cmd)
{
  auto [status, sent_requests] =
    this->mServices.rel_service->getSentFriendRequests(cmd.user_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command GetFriendRequests return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("Sending sent friend requests list to %1").arg(cmd.user_id);
  }
  GetSentFriendRequestsResponse gsfrr;
  gsfrr.client_id = cmd.client_id;
  gsfrr.status = status;
  gsfrr.sent_requests = sent_requests;
  return std::vector<Response>{ gsfrr };
}

std::vector<Response>
Dispatcher::handleGetServerStats(const GetServerStats& cmd)
{
  const auto [status, total] = this->mServices.u_service->countUsers();
  const unsigned int online{ this->mRegistry->totalOnline() };
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command GetServerStats return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("Server stats: %1/%2").arg(online).arg(total);
  }
  GetServerStatsResponse gssr;
  gssr.client_id = cmd.client_id;
  gssr.status = status;
  gssr.online = online;
  gssr.total = total;
  return { gssr };
}

std::vector<Response>
Dispatcher::handleStartCall(const StartCall& cmd)
{
  StartCallResponse scr;
  scr.client_id = cmd.client_id;
  scr.call_id = QUuid::createUuid();

  if (cmd.callee_id == cmd.user_id) {
    qWarning(appDispatcher)
      << QString("User %1 tried to call themselves").arg(cmd.user_id);
    scr.status = OperationStatus::CallWithYourself;
    return std::vector<Response>{ scr };
  }

  OperationStatus status =
    this->mServices.rel_service->areFriends(cmd.user_id, cmd.callee_id);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command StartCall return status %1").arg((int)status);
    scr.status = status;
    return std::vector<Response>{ scr };
  }

  std::optional<QUuid> callee_client_id =
    this->mRegistry->getClientId(cmd.callee_id);
  if (!callee_client_id.has_value()) {
    qInfo(appDispatcher)
      << QString("User %1 is offline, can't start a call").arg(cmd.callee_id);
    scr.status = OperationStatus::UserOffline;
    return std::vector<Response>{ scr };
  }

  bool isCall = this->mCallRegistry->isUserInCall(cmd.user_id) ||
                this->mCallRegistry->isUserInCall(cmd.callee_id);
  if (isCall) {
    qWarning(appDispatcher) << QString("User %1 or %2 already in call")
                                 .arg(cmd.user_id)
                                 .arg(cmd.callee_id);
    scr.status = OperationStatus::UserAlreadyInCall;
    return std::vector<Response>{ scr };
  }

  QUuid call_id = this->mCallRegistry->createRecord(cmd.user_id, cmd.callee_id);
  qInfo(appDispatcher) << QString("Call %1 created: %2 -> %3")
                            .arg(call_id.toString())
                            .arg(cmd.user_id)
                            .arg(cmd.callee_id);

  scr.status = OperationStatus::OK;
  scr.call_id = call_id;

  IncomingCallResponse icr;
  icr.client_id = callee_client_id.value();
  icr.call_id = call_id;
  icr.caller_id = cmd.user_id;
  icr.with_video = cmd.with_video;

  return std::vector<Response>{ scr, icr };
}

std::vector<Response>
Dispatcher::handleAcceptCall(const AcceptCall& cmd)
{
  AcceptCallResponse acr;
  acr.client_id = cmd.client_id;
  acr.call_id = cmd.call_id;

  std::optional<CallRecord> record =
    this->mCallRegistry->getCallRecord(cmd.call_id);
  if (!record.has_value()) {
    qWarning(appDispatcher)
      << QString("Call %1 doesn't exist").arg(cmd.call_id.toString());
    acr.status = OperationStatus::NoSuchCall;
    return std::vector<Response>{ acr };
  }
  if (record->callee_id != cmd.user_id) {
    qWarning(appDispatcher) << QString("User %1 is not callee of call %2")
                                 .arg(cmd.user_id)
                                 .arg(cmd.call_id.toString());
    acr.status = OperationStatus::NotCallParticipant;
    return std::vector<Response>{ acr };
  } else if (record->status != CallStatus::Ringing) {
    qWarning(appDispatcher) << QString("Calling already accepted");
    acr.status = OperationStatus::CallAlreadyProceeded;
    return std::vector<Response>{ acr };
  }

  this->mCallRegistry->updateRecord(cmd.call_id, CallStatus::Active);
  acr.status = OperationStatus::OK;
  std::vector<Response> responses{ acr };

  std::optional<QUuid> caller_client_id =
    this->mRegistry->getClientId(record->caller_id);
  if (caller_client_id.has_value()) {
    CallAcceptedResponse car;
    car.client_id = caller_client_id.value();
    car.call_id = cmd.call_id;
    responses.push_back(car);
  } else {
    qWarning(appDispatcher)
      << QString("Caller %1 is no longer online").arg(record->caller_id);
  }

  return responses;
}

std::vector<Response>
Dispatcher::handleRejectCall(const RejectCall& cmd)
{
  RejectCallResponse rcr;
  rcr.client_id = cmd.client_id;
  rcr.call_id = cmd.call_id;

  std::optional<CallRecord> record =
    this->mCallRegistry->getCallRecord(cmd.call_id);
  if (!record.has_value()) {
    qWarning(appDispatcher)
      << QString("Call %1 doesn't exist").arg(cmd.call_id.toString());
    rcr.status = OperationStatus::NoSuchCall;
    return std::vector<Response>{ rcr };
  }
  if (record->callee_id != cmd.user_id) {
    qWarning(appDispatcher) << QString("User %1 is not callee of call %2")
                                 .arg(cmd.user_id)
                                 .arg(cmd.call_id.toString());
    rcr.status = OperationStatus::NotCallParticipant;
    return std::vector<Response>{ rcr };
  } else if (record->status != CallStatus::Ringing) {
    qWarning(appDispatcher)
      << QString("Call %1 already accepted").arg(cmd.call_id.toString());
    rcr.status = OperationStatus::CallAlreadyProceeded;
    return std::vector<Response>{ rcr };
  }

  this->mCallRegistry->deleteRecord(cmd.call_id);
  rcr.status = OperationStatus::OK;
  std::vector<Response> responses{ rcr };

  std::optional<QUuid> caller_client_id =
    this->mRegistry->getClientId(record->caller_id);
  if (caller_client_id.has_value()) {
    CallRejectedResponse crr;
    crr.client_id = caller_client_id.value();
    crr.call_id = cmd.call_id;
    responses.push_back(crr);
  }

  return responses;
}

std::vector<Response>
Dispatcher::handleEndCall(const EndCall& cmd)
{
  EndCallResponse ecr;
  ecr.client_id = cmd.client_id;
  ecr.call_id = cmd.call_id;

  std::optional<CallRecord> record =
    this->mCallRegistry->getCallRecord(cmd.call_id);
  if (!record.has_value()) {
    qWarning(appDispatcher)
      << QString("Call %1 doesn't exist").arg(cmd.call_id.toString());
    ecr.status = OperationStatus::NoSuchCall;
    return std::vector<Response>{ ecr };
  }
  if (record->caller_id != cmd.user_id && record->callee_id != cmd.user_id) {
    qWarning(appDispatcher)
      << QString("User %1 is not a participant of call %2")
           .arg(cmd.user_id)
           .arg(cmd.call_id.toString());
    ecr.status = OperationStatus::NotCallParticipant;
    return std::vector<Response>{ ecr };
  }

  this->mCallRegistry->deleteRecord(cmd.call_id);
  ecr.status = OperationStatus::OK;
  std::vector<Response> responses{ ecr };

  unsigned int other_id =
    (cmd.user_id == record->caller_id) ? record->callee_id : record->caller_id;
  std::optional<QUuid> other_client_id = this->mRegistry->getClientId(other_id);
  if (other_client_id.has_value()) {
    CallEndedResponse cer;
    cer.client_id = other_client_id.value();
    cer.call_id = cmd.call_id;
    responses.push_back(cer);
  }

  return responses;
}

std::vector<Response>
Dispatcher::handleSdp(const Sdp& cmd)
{
  std::optional<CallRecord> record =
    this->mCallRegistry->getCallRecord(cmd.call_id);
  if (!record.has_value()) {
    qWarning(appDispatcher)
      << QString("Call %1 doesn't exist").arg(cmd.call_id.toString());
    SdpResponse sr;
    sr.client_id = cmd.client_id;
    sr.call_id = cmd.call_id;
    sr.sdp = "";
    sr.status = OperationStatus::NoSuchCall;
    return std::vector<Response>{ sr };
  }
  if (record->caller_id != cmd.user_id && record->callee_id != cmd.user_id) {
    qWarning(appDispatcher)
      << QString("User %1 is not a participant of call %2")
           .arg(cmd.user_id)
           .arg(cmd.call_id.toString());
    SdpResponse sr;
    sr.client_id = cmd.client_id;
    sr.call_id = cmd.call_id;
    sr.sdp = "";
    sr.status = OperationStatus::NotCallParticipant;
    return std::vector<Response>{ sr };
  } else if (record->status != CallStatus::Active) {
    qWarning(appDispatcher)
      << QString("Call %1 not established").arg(cmd.client_id.toString());
    SdpResponse sr;
    sr.client_id = cmd.client_id;
    sr.call_id = cmd.call_id;
    sr.sdp = "";
    sr.status = OperationStatus::CallNotEstablished;
    return std::vector<Response>{ sr };
  }

  unsigned int other_id =
    (cmd.user_id == record->caller_id) ? record->callee_id : record->caller_id;
  std::optional<QUuid> other_client_id = this->mRegistry->getClientId(other_id);
  if (!other_client_id.has_value()) {
    qWarning(appDispatcher) << QString("Other party %1 of call %2 is offline")
                                 .arg(other_id)
                                 .arg(cmd.call_id.toString());
    SdpResponse sr;
    sr.client_id = cmd.client_id;
    sr.call_id = cmd.call_id;
    sr.sdp = "";
    sr.status = OperationStatus::UserOffline;
    return std::vector<Response>{ sr };
  }

  SdpResponse sr;
  sr.client_id = other_client_id.value();
  sr.call_id = cmd.call_id;
  sr.sdp = cmd.sdp;
  sr.status = OperationStatus::OK;
  return std::vector<Response>{ sr };
}

std::vector<Response>
Dispatcher::handleIceCandidate(const IceCandidate& cmd)
{
  std::optional<CallRecord> record =
    this->mCallRegistry->getCallRecord(cmd.call_id);
  if (!record.has_value()) {
    qWarning(appDispatcher)
      << QString("Call %1 doesn't exist").arg(cmd.call_id.toString());
    IceCandidateResponse icr;
    icr.client_id = cmd.client_id;
    icr.call_id = cmd.call_id;
    icr.candidate = "";
    icr.mid = "";
    icr.status = OperationStatus::NoSuchCall;
    return std::vector<Response>{ icr };
  }
  if (record->caller_id != cmd.user_id && record->callee_id != cmd.user_id) {
    qWarning(appDispatcher)
      << QString("User %1 is not a participant of call %2")
           .arg(cmd.user_id)
           .arg(cmd.call_id.toString());
    IceCandidateResponse icr;
    icr.client_id = cmd.client_id;
    icr.call_id = cmd.call_id;
    icr.candidate = "";
    icr.mid = "";
    icr.status = OperationStatus::NotCallParticipant;
    return std::vector<Response>{ icr };
  } else if (record->status != CallStatus::Active) {
    qWarning(appDispatcher)
      << QString("Call %1 not established").arg(cmd.client_id.toString());
    IceCandidateResponse icr;
    icr.client_id = cmd.client_id;
    icr.call_id = cmd.call_id;
    icr.candidate = "";
    icr.mid = "";
    icr.status = OperationStatus::CallNotEstablished;
    return std::vector<Response>{ icr };
  }

  unsigned int other_id =
    (cmd.user_id == record->caller_id) ? record->callee_id : record->caller_id;
  std::optional<QUuid> other_client_id = this->mRegistry->getClientId(other_id);
  if (!other_client_id.has_value()) {
    qWarning(appDispatcher) << QString("Other party %1 of call %2 is offline")
                                 .arg(other_id)
                                 .arg(cmd.call_id.toString());
    IceCandidateResponse icr;
    icr.client_id = cmd.client_id;
    icr.call_id = cmd.call_id;
    icr.candidate = "";
    icr.mid = "";
    icr.status = OperationStatus::UserOffline;
    return std::vector<Response>{ icr };
  }

  IceCandidateResponse icr;
  icr.client_id = other_client_id.value();
  icr.call_id = cmd.call_id;
  icr.candidate = cmd.candidate;
  icr.mid = cmd.mid;
  icr.status = OperationStatus::OK;
  return std::vector<Response>{ icr };
}

std::vector<Response>
Dispatcher::handleGetTurnCredentials(const GetTurnCredentials& cmd)
{
  GetTurnCredentialsResponse response;
  response.client_id = cmd.client_id;
  response.status = OperationStatus::OK;

  if (mTurnSecret.isEmpty()) {
    response.status = OperationStatus::InternalError;
    response.username = "";
    response.password = "";
    response.ttl = 0;
    qCritical(appDispatcher)
      << "TURN secret not configured, cannot generate credentials";
    return std::vector<Response>{ response };
  }

  int ttl = 3600;
  qint64 expiry = QDateTime::currentSecsSinceEpoch() + ttl;
  QString username = QString::number(expiry);

  QByteArray secret = mTurnSecret.toUtf8();
  QByteArray msg = username.toUtf8();
  QByteArray hmac =
    QMessageAuthenticationCode::hash(msg, secret, QCryptographicHash::Sha1);
  QString password = hmac.toHex();

  response.username = username;
  response.password = password;
  response.ttl = ttl;
  response.status = OperationStatus::OK;

  qInfo(appDispatcher) << "Generated TURN credentials for user" << cmd.user_id
                       << "expiring at" << expiry;

  return std::vector<Response>{ response };
}

std::vector<Response>
Dispatcher::handleSetAvatar(const SetAvatar& cmd)
{
  OperationStatus status =
    this->mServices.u_service->setAvatar(cmd.user_id, cmd.avatar);
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command SetAvatar return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("Avatar updated for user %1").arg(cmd.user_id);
  }
  SetAvatarResponse sar;
  sar.client_id = cmd.client_id;
  sar.status = status;
  return std::vector<Response>{ sar };
}

std::vector<Response>
Dispatcher::handleGetOnlineUsers(const GetOnlineUsers& cmd)
{
  GetOnlineUsersResponse gour;
  gour.client_id = cmd.client_id;

  std::vector<unsigned int> ids = this->mRegistry->getOnlineUserIds();
  if (ids.empty()) {
    qInfo(appDispatcher) << "No users online";
    gour.status = OperationStatus::OK;
    gour.users = std::nullopt;
    return std::vector<Response>{ gour };
  }

  auto [status, users] = this->mServices.u_service->getUsersByID(ids);
  gour.status = status;
  gour.users = users;
  if (status != OperationStatus::OK) {
    qWarning(appDispatcher)
      << QString("Command GetOnlineUsers return status %1").arg((int)status);
  } else {
    qInfo(appDispatcher)
      << QString("Sending online users list to %1").arg(cmd.user_id);
  }
  return std::vector<Response>{ gour };
}