#pragma once
#include "structures.h"

#include <QString>
#include <QUuid>

namespace wire {

/**
 * @brief Ответ на регистрацию пользователя
 * @see RegisterUser
 * @see OperationStatus
 */
struct RegisterUserResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на вход пользователя
 * @see LoginUser
 * @see OperationStatus
 */
struct LoginUserResponse
{
  unsigned int user_id;
  OperationStatus status;
};

/**
 * @brief Ответ на отправленное письмо. Нужно для понимания, доставлено письмо
 * вообще или нет
 * @see SendMessage
 * @see OperationStatus
 */
struct SendMessageResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на новое письмо пользователю. Отправляется, когда адресат
 * находится в сети
 * @see OperationStatus
 */
struct NewMessageResponse
{
  unsigned int sender_id;
  QString content;
};

/**
 * @brief Ответ на отправку заявки в друзья
 */
struct SendFriendRequestResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на принятие заявки в друзья
 */
struct AcceptFriendRequestResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на отклонение заявки в друзья
 */
struct RejectFriendRequestResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на удаление из друзей
 */
struct RemoveFriendResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на получение списка друзей
 */
struct GetFriendsResponse
{
  OperationStatus status;
  std::optional<std::vector<User>>
    friends; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение списка заявок в друзья
 */
struct GetFriendRequestsResponse
{
  OperationStatus status;
  std::optional<std::vector<User>>
    requests; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение списка отправленных заявок в друзья
 */
struct GetSentFriendRequestsResponse
{
  OperationStatus status;
  std::optional<std::vector<User>>
    sent_requests; /**< Список пользователей, может быть std::nullopt */
};

/**
 * @brief Ответ на получение статистики с сервера - число
 * онлайн-пользователей и общее число пользователей
 */
struct GetServerStatsResponse
{
  OperationStatus status;
  unsigned int online;
  unsigned int total;
};

/**
 * @brief Ответ инициатору звонка на команду StartCall
 * @see StartCall
 * @see OperationStatus
 */
struct StartCallResponse
{
  OperationStatus status;
  QUuid call_id; /**< Валиден только при status == OK */
};

/**
 * @brief Уведомление о входящем звонке. Отправляется вызываемому в момент
 * StartCall, если он в сети
 */
struct IncomingCallResponse
{
  QUuid call_id;
  unsigned int caller_id;
  bool with_video;
};

/**
 * @brief Ответ вызываемому на команду AcceptCall
 * @see AcceptCall
 * @see OperationStatus
 */
struct AcceptCallResponse
{
  OperationStatus status;
  QUuid call_id;
};

/**
 * @brief Уведомление инициатору о том, что вызываемый принял звонок
 */
struct CallAcceptedResponse
{
  QUuid call_id;
};

/**
 * @brief Ответ вызываемому на команду RejectCall
 * @see RejectCall
 * @see OperationStatus
 */
struct RejectCallResponse
{
  OperationStatus status;
  QUuid call_id;
};

/**
 * @brief Уведомление инициатору о том, что вызываемый отклонил звонок
 */
struct CallRejectedResponse
{
  QUuid call_id;
};

/**
 * @brief Ответ на команду EndCall тому, кто завершил звонок
 * @see EndCall
 * @see OperationStatus
 */
struct EndCallResponse
{
  OperationStatus status;
  QUuid call_id;
};

/**
 * @brief Уведомление второй стороне о том, что звонок завершен
 */
struct CallEndedResponse
{
  QUuid call_id;
};

/**
 * @brief Ретрансляция SDP (offer/answer) второй стороне звонка.
 * Также используется как ответ отправителю при ошибке (тогда call_id
 * содержит переданный call_id, а sdp пустая строка)
 * @see Sdp
 */
struct SdpResponse
{
  QUuid call_id;
  QString sdp;
  OperationStatus status; /**< OK — при успешной ретрансляции второй стороне;
                           NoSuchCall, NotCallParticipant, CallNotEstablished,
                           UserOffline или InternalError — при ошибке (тогда
                           ответ уходит отправителю, а не второй стороне) */
};

/**
 * @brief Ретрансляция ICE-кандидата второй стороне звонка. Также
 * используется как ответ отправителю при ошибке (см. SdpResponse)
 * @see IceCandidate
 */
struct IceCandidateResponse
{
  QUuid call_id;
  QString candidate;
  QString mid;
  OperationStatus status;
};

struct GetTurnCredentialsResponse
{
  QString username;   // временный логин (Unix timestamp)
  QString password;   // HMAC-SHA1(secret, username) в hex
  OperationStatus status;
  int ttl;
};

/**
 * @brief Ответ на установку/обновление аватарки
 */
struct SetAvatarResponse
{
  OperationStatus status;
};

/**
 * @brief Ответ на получение списка всех онлайн-пользователей
 */
struct GetOnlineUsersResponse
{
  OperationStatus status;
  std::optional<std::vector<User>>
    users; /**< Список онлайн-пользователей, может быть std::nullopt */
};

}