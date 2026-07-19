#pragma once
#include "context.h"
#include "responses.h"

#include <QString>
#include <QUuid>

/**
 * @brief Ответ на регистрацию пользователя
 * @see RegisterUser
 * @see OperationStatus
 */
struct RegisterUserResponse
  : public wire::RegisterUserResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на вход пользователя
 * @see LoginUser
 * @see OperationStatus
 */
struct LoginUserResponse
  : public wire::LoginUserResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на отправленное письмо. Нужно для понимания, доставлено письмо
 * вообще или нет
 * @see SendMessage
 * @see OperationStatus
 */
struct SendMessageResponse
  : public wire::SendMessageResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на новое письмо пользователю. Отправляется, когда адресат
 * находится в сети
 * @see OperationStatus
 */
struct NewMessageResponse
  : public wire::NewMessageResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на отправку заявки в друзья
 */
struct SendFriendRequestResponse
  : public wire::SendFriendRequestResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на принятие заявки в друзья
 */
struct AcceptFriendRequestResponse
  : public wire::AcceptFriendRequestResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на отклонение заявки в друзья
 */
struct RejectFriendRequestResponse
  : public wire::RejectFriendRequestResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на удаление из друзей
 */
struct RemoveFriendResponse
  : public wire::RemoveFriendResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение списка друзей
 */
struct GetFriendsResponse
  : public wire::GetFriendsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение списка заявок в друзья
 */
struct GetFriendRequestsResponse
  : public wire::GetFriendRequestsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение списка отправленных заявок в друзья
 */
struct GetSentFriendRequestsResponse
  : public wire::GetSentFriendRequestsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на получение статистики с сервера - число
 * онлайн-пользователей и общее число пользователей
 */
struct GetServerStatsResponse
  : public wire::GetServerStatsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ инициатору звонка на команду StartCall
 */
struct StartCallResponse
  : public wire::StartCallResponse
  , public ClientCtx
{};

/**
 * @brief Уведомление о входящем звонке вызываемому
 */
struct IncomingCallResponse
  : public wire::IncomingCallResponse
  , public ClientCtx
{};

/**
 * @brief Ответ вызываемому на команду AcceptCall
 */
struct AcceptCallResponse
  : public wire::AcceptCallResponse
  , public ClientCtx
{};

/**
 * @brief Уведомление инициатору о принятии звонка
 */
struct CallAcceptedResponse
  : public wire::CallAcceptedResponse
  , public ClientCtx
{};

/**
 * @brief Ответ вызываемому на команду RejectCall
 */
struct RejectCallResponse
  : public wire::RejectCallResponse
  , public ClientCtx
{};

/**
 * @brief Уведомление инициатору об отклонении звонка
 */
struct CallRejectedResponse
  : public wire::CallRejectedResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на команду EndCall тому, кто завершил звонок
 */
struct EndCallResponse
  : public wire::EndCallResponse
  , public ClientCtx
{};

/**
 * @brief Уведомление второй стороне о завершении звонка
 */
struct CallEndedResponse
  : public wire::CallEndedResponse
  , public ClientCtx
{};

/**
 * @brief Ретрансляция SDP второй стороне звонка
 */
struct SdpResponse
  : public wire::SdpResponse
  , public ClientCtx
{};

/**
 * @brief Ретрансляция ICE-кандидата второй стороне звонка
 */
struct IceCandidateResponse
  : public wire::IceCandidateResponse
  , public ClientCtx
{};

struct GetTurnCredentialsResponse
  : public wire::GetTurnCredentialsResponse
  , public ClientCtx
{};

/**
 * @brief Ответ на установку/обновление аватарки
 */
struct SetAvatarResponse
  : public wire::SetAvatarResponse
  , public ClientCtx
{};
 
/**
 * @brief Ответ на получение списка всех онлайн-пользователей
 */
struct GetOnlineUsersResponse
  : public wire::GetOnlineUsersResponse
  , public ClientCtx
{};
