# WebSocket Protocol v1
## 1. Общий формат сообщения

Любое сообщение — JSON-объект с двумя полями:

```json
{
  "type": "<command_or_response_name>",
  "payload": { ... }
}
```

- `type` — строка, идентифицирующая команду/ответ (snake_case).
- `payload` — объект с данными, специфичными для конкретного типа.

!!ВНИМАНИЕ. Сервер принимает только бинарный формат, поэтому перед отправкой json убедитесь, что он превращен в байты!!


### Типы данных

| C++ тип         | JSON тип                          |
|-----------------|------------------------------------|
| `QString`       | `string`                           |
| `unsigned int`  | `number` (целое, ≥ 0)              |
| `OperationStatus` | `number` (целое, код статуса — см. таблицу ниже) |
| `User`          | `object` (`{ "user_id": number, "login": string, "avatar": string, "last_seen": string \| null }`) |
| `std::vector<User>` (опционально) | `array` объектов `User`; если данных нет — пустой массив `[]` |

`User.avatar` — аватарка пользователя в base64 (см. [`set_avatar`](#212-set_avatar--установкаобновление-аватарки)); пустая строка, если не установлена.
`User.last_seen` — время последней активности пользователя в формате ISO 8601 (`Qt::ISODate`); `null`, если пользователь еще ни разу не проявлял активность.

### Коды `status` (`OperationStatus`)

| Код   | Значение               | Описание                                                        |
|-------|-------------------------|-------------------------------------------------------------------|
| `0`   | `OK`                    | Операция выполнена успешно                                       |
| `1`   | `UserExist`             | При регистрации логин уже занят                                  |
| `2`   | `UserNotExist`          | Пользователь с таким логином/id не найден                        |
| `3`   | `InvalidCredentials`    | Неверный логин или пароль (при входе)                             |
| `4`   | `RelationAlreadyExist`  | Заявка в друзья / дружба уже существует                          |
| `5`   | `RelationWithYourself`  | Попытка отправить заявку/добавить в друзья самого себя           |
| `6`   | `NoSuchRelation`        | Нет такой заявки/связи (например, принять несуществующую заявку) |
| `7`   | `UserNotInFriends`      | Попытка удалить из друзей того, кто не в друзьях                 |
| `8`   | `UserOffline`           | Вызываемый пользователь / собеседник по звонку не в сети          |
| `9`   | `NoSuchCall`            | Звонка с таким `call_id` не существует                           |
| `10`  | `NotCallParticipant`    | Пользователь не является участником данного звонка               |
| `11`  | `CallWithYourself`      | Попытка позвонить самому себе                                     |
| `12`  | `UserAlreadyInCall`     | Инициатор или вызываемый уже участвует в другом звонке            |
| `13`  | `CallAlreadyProceeded`  | Звонок уже принят/отклонен/завершен ранее                        |
| `14`  | `InvalidAvatar`         | Присланные данные не являются валидным изображением, либо превышен допустимый размер |
| `15`  | `CallNotEstablished`    | Звонок существует, но еще не находится в статусе `Active` (например, еще не принят) |
| `255` | `InternalError`         | Внутренняя ошибка сервера                                         |

---

## 2. Команды (клиент → сервер)

### 2.1 `register_user` — регистрация пользователя

```json
{
  "type": "register_user",
  "payload": {
    "login": "string",
    "pwd": "string"
  }
}
```

**Ответ:** [`register_user_response`](#31-register_user_response)

---

### 2.2 `login_user` — вход пользователя

```json
{
  "type": "login_user",
  "payload": {
    "login": "string",
    "pwd": "string"
  }
}
```

**Ответ:** [`login_user_response`](#32-login_user_response)

---

### 2.3 `send_message` — отправка сообщения

```json
{
  "type": "send_message",
  "payload": {
    "receiver_id": 1,
    "content": "string"
  }
}
```

**Ответ:** [`send_message_response`](#33-send_message_response) — подтверждение доставки.
Если получатель онлайн, ему дополнительно приходит
[`new_message`](#37-new_message).

---

### 2.4 `send_friend_request` — заявка в друзья

```json
{
  "type": "send_friend_request",
  "payload": {
    "friend_id": 2
  }
}
```

**Ответ:** [`send_friend_request_response`](#34-send_friend_request_response)

---

### 2.5 `accept_friend_request` — принятие заявки в друзья

```json
{
  "type": "accept_friend_request",
  "payload": {
    "friend_id": 2
  }
}
```

**Ответ:** [`accept_friend_request_response`](#35-accept_friend_request_response)

---

### 2.6 `reject_friend_request` — отклонение заявки в друзья

```json
{
  "type": "reject_friend_request",
  "payload": {
    "friend_id": 2
  }
}
```

**Ответ:** [`reject_friend_request_response`](#36-reject_friend_request_response)

---

### 2.7 `remove_friend` — удаление из друзей

```json
{
  "type": "remove_friend",
  "payload": {
    "friend_id": 2
  }
}
```

**Ответ:** [`remove_friend_response`](#38-remove_friend_response)

---

### 2.8 `get_friends` — получение списка друзей

```json
{
  "type": "get_friends",
  "payload": {}
}
```

**Ответ:** [`get_friends_response`](#39-get_friends_response)

---

### 2.9 `get_friend_requests` — получение входящих заявок в друзья

```json
{
  "type": "get_friend_requests",
  "payload": {}
}
```

**Ответ:** [`get_friend_requests_response`](#310-get_friend_requests_response)

---

### 2.10 `get_sent_friend_requests` — получение исходящих заявок в друзья

```json
{
  "type": "get_sent_friend_requests",
  "payload": {}
}
```

**Ответ:** [`get_sent_friend_requests_response`](#311-get_sent_friend_requests_response)

---

### 2.11 `get_server_stats` — получение статистики сервера

```json
{
  "type": "get_server_stats",
  "payload": {}
}
```

**Ответ:** [`get_server_stats_response`](#312-get_server_stats_response)

---

### 2.12 `get_turn_credentials` — получение временных учетных данных TURN-сервера

Требует авторизации.

```json
{
  "type": "get_turn_credentials",
  "payload": {}
}
```

**Ответ:** [`get_turn_credentials_response`](#313-get_turn_credentials_response)

---

### 2.13 `set_avatar` — установка/обновление аватарки

Требует авторизации. `avatar` — изображение в формате PNG, JPEG, GIF или WEBP,
закодированное в base64. Декодированный размер не должен превышать 1 МБ,
иначе вернется статус `InvalidAvatar` (14).

```json
{
  "type": "set_avatar",
  "payload": {
    "avatar": "base64-encoded-image-data..."
  }
}
```

**Ответ:** [`set_avatar_response`](#314-set_avatar_response)

---

### 2.14 `get_online_users` — получение списка всех онлайн-пользователей

Требует авторизации.

```json
{
  "type": "get_online_users",
  "payload": {}
}
```

**Ответ:** [`get_online_users_response`](#315-get_online_users_response)

---

## 3. Ответы (сервер → клиент)

### 3.1 `register_user_response`

Возможные статусы: `OK` (0), `UserExist` (1), `InternalError` (255).

```json
{
  "type": "register_user_response",
  "payload": {
    "status": 0
  }
}
```

### 3.2 `login_user_response`

Возможные статусы: `OK` (0), `UserNotExist` (2), `InvalidCredentials` (3),
`InternalError` (255). При статусе, отличном от `OK`, поле `user_id` имеет значение `0`.

```json
{
  "type": "login_user_response",
  "payload": {
    "user_id": 1,
    "status": 0
  }
}
```

### 3.3 `send_message_response`

Возможные статусы: `OK` (0), `UserNotExist` (2) — если `receiver_id` не
существует, `InternalError` (255).

```json
{
  "type": "send_message_response",
  "payload": {
    "status": 0
  }
}
```

### 3.4 `send_friend_request_response`

Возможные статусы: `OK` (0), `UserNotExist` (2), `RelationAlreadyExist` (4),
`RelationWithYourself` (5), `InternalError` (255).

```json
{
  "type": "send_friend_request_response",
  "payload": {
    "status": 0
  }
}
```

### 3.5 `accept_friend_request_response`

Возможные статусы: `OK` (0), `NoSuchRelation` (6) — если заявки не было,
`InternalError` (255).

```json
{
  "type": "accept_friend_request_response",
  "payload": {
    "status": 0
  }
}
```

### 3.6 `reject_friend_request_response`

Возможные статусы: `OK` (0), `NoSuchRelation` (6), `InternalError` (255).

```json
{
  "type": "reject_friend_request_response",
  "payload": {
    "status": 0
  }
}
```

### 3.7 `new_message` — входящее сообщение (не является ответом на конкретную команду)

Отправляется получателю сообщения сервером в момент, когда он онлайн.

```json
{
  "type": "new_message",
  "payload": {
    "sender_id": 1,
    "content": "string"
  }
}
```

### 3.8 `remove_friend_response`

Возможные статусы: `OK` (0), `UserNotInFriends` (7), `InternalError` (255).

```json
{
  "type": "remove_friend_response",
  "payload": {
    "status": 0
  }
}
```

### 3.9 `get_friends_response`

Возможные статусы: `OK` (0), `InternalError` (255). Поле `friends` — массив
объектов `User`; при статусе, отличном от `OK`, будет пустым массивом.

```json
{
  "type": "get_friends_response",
  "payload": {
    "status": 0,
    "friends": [
      { "user_id": 2, "login": "alice", "avatar": "", "last_seen": "2026-07-19T10:15:00" },
      { "user_id": 3, "login": "bob", "avatar": "iVBORw0KGgo...", "last_seen": null }
    ]
  }
}
```

### 3.10 `get_friend_requests_response`

Возможные статусы: `OK` (0), `InternalError` (255). Поле `requests` —
массив пользователей, приславших входящую заявку в друзья.

```json
{
  "type": "get_friend_requests_response",
  "payload": {
    "status": 0,
    "requests": [
      { "user_id": 4, "login": "carol", "avatar": "", "last_seen": "2026-07-18T21:03:00" }
    ]
  }
}
```

### 3.11 `get_sent_friend_requests_response`

Возможные статусы: `OK` (0), `InternalError` (255). Поле `sent_requests` —
массив пользователей, которым текущий пользователь отправил заявку в друзья.

```json
{
  "type": "get_sent_friend_requests_response",
  "payload": {
    "status": 0,
    "sent_requests": [
      { "user_id": 5, "login": "dave", "avatar": "", "last_seen": null }
    ]
  }
}
```

### 3.12 `get_server_stats_response`

Возможные статусы: `OK` (0), `InternalError` (255). `online` - количество пользователей онлайн, `total` - общее число пользователей

```json
{
  "type": "get_server_stats_response",
  "payload": {
    "status": 0,
    "online": 1,
    "total": 2
  }
}
```

### 3.13 `get_turn_credentials_response`

Возможные статусы: `OK` (0), `InternalError` (255) — если TURN-секрет не
настроен на сервере. `username`/`password` — временные учетные данные для
TURN-сервера (схема REST API, HMAC-SHA1), `ttl` — время их жизни в секундах.

```json
{
  "type": "get_turn_credentials_response",
  "payload": {
    "status": 0,
    "username": "1700000000",
    "password": "a1b2c3...",
    "ttl": 3600
  }
}
```

### 3.14 `set_avatar_response`

Возможные статусы: `OK` (0), `InvalidAvatar` (14) — данные не являются
валидным изображением или превышают 1 МБ, `UserNotExist` (2),
`InternalError` (255).

```json
{
  "type": "set_avatar_response",
  "payload": {
    "status": 0
  }
}
```

### 3.15 `get_online_users_response`

Возможные статусы: `OK` (0), `InternalError` (255). Поле `users` — массив
всех онлайн-пользователей (включая себя); пустой массив, если онлайн никого
нет.

```json
{
  "type": "get_online_users_response",
  "payload": {
    "status": 0,
    "users": [
      { "user_id": 1, "login": "alice", "avatar": "", "last_seen": "2026-07-19T12:00:00" }
    ]
  }
}
```

### 3.16 `error`

Возвращает ошибку, непредусмотренную в `OperationStatus` — то есть
сетевые/протокольные сбои, не относящиеся к бизнес-логике конкретной
команды: невалидный JSON, отсутствие обязательного поля, неизвестный `type`,
попытка выполнить команду, требующую авторизации, без нее, превышение
допустимого размера сообщения, а также блокировки за флуд/брутфорс.

Ошибки бизнес-логики (например, "звонок не найден" при `sdp`/`ice_candidate`)
теперь возвращаются с соответствующим `OperationStatus` в самом ответе
команды (см. [4.5](#45-sdp--обмен-sdp-offeranswer) и
[4.6](#46-ice_candidate--обмен-ice-кандидатами)), а не через `error`.

```json
{
  "type": "error",
  "payload": {
    "reason": "string"
  }
}
```

---

## 4. Звонки (WebRTC signaling)

Звонок идентифицируется по `call_id` (UUID-строка без фигурных скобок,
например `"550e8400-e29b-41d4-a716-446655440000"`), который сервер выдаёт
в ответ на `start_call`. Позвонить можно только пользователю, который
находится в друзьях у инициатора и в сети.

### 4.1 `start_call` — инициировать звонок

```json
{
  "type": "start_call",
  "payload": { "callee_id": 2, "with_video": false }
}
```

**Ответ:** [`start_call_response`](#47-start_call_response) инициатору.
Если `status == OK`, вызываемому дополнительно приходит
[`incoming_call`](#48-incoming_call).

### 4.2 `accept_call` — принять звонок

```json
{
  "type": "accept_call",
  "payload": { "call_id": "..." }
}
```

**Ответ:** [`accept_call_response`](#49-accept_call_response) вызываемому.
Если `status == OK`, инициатору (если он ещё в сети) приходит
[`call_accepted`](#410-call_accepted).

### 4.3 `reject_call` — отклонить звонок

```json
{
  "type": "reject_call",
  "payload": { "call_id": "..." }
}
```

**Ответ:** [`reject_call_response`](#411-reject_call_response) вызываемому.
Если `status == OK`, инициатору (если он ещё в сети) приходит
[`call_rejected`](#412-call_rejected).

### 4.4 `end_call` — завершить звонок

Может отправить любая из сторон звонка.

```json
{
  "type": "end_call",
  "payload": { "call_id": "..." }
}
```

**Ответ:** [`end_call_response`](#413-end_call_response) отправителю.
Если `status == OK`, второй стороне (если она ещё в сети) приходит
[`call_ended`](#414-call_ended).

### 4.5 `sdp` — обмен SDP (offer/answer)

```json
{
  "type": "sdp",
  "payload": { "call_id": "...", "sdp": "v=0..." }
}
```

При успехе ретранслируется без изменений второй стороне звонка как
[`sdp`](#415-sdp-1) со `status: 0`. Если звонок не найден, отправитель не
участник звонка, звонок еще не установлен, либо собеседник не в сети —
ответ с соответствующим статусом (`NoSuchCall` (9), `NotCallParticipant`
(10), `CallNotEstablished` (15) или `UserOffline` (8)) приходит не
собеседнику, а самому отправителю, тем же типом `sdp` (поле `sdp` в этом
случае пустое).

### 4.6 `ice_candidate` — обмен ICE-кандидатами

```json
{
  "type": "ice_candidate",
  "payload": { "call_id": "...", "candidate": "candidate:...", "mid": "0" }
}
```

Ретранслируется второй стороне звонка как
[`ice_candidate`](#416-ice_candidate-1) со `status: 0`. Правила ошибок — как
у `sdp` (ответ с ненулевым статусом уходит отправителю, поля `candidate` и
`mid` в этом случае пустые).

---

### 4.7 `start_call_response`

Возможные статусы: `OK` (0), `CallWithYourself` (11), `UserNotInFriends` (7),
`UserOffline` (8), `UserAlreadyInCall` (12), `InternalError` (255). Поле
`call_id` валидно только при `status == OK`.

```json
{
  "type": "start_call_response",
  "payload": { "status": 0, "call_id": "550e8400-e29b-41d4-a716-446655440000" }
}
```

### 4.8 `incoming_call`

Уведомление о входящем звонке. Отправляется вызываемому в момент
`start_call`, если он в сети.

```json
{
  "type": "incoming_call",
  "payload": { "call_id": "...", "caller_id": 1 }
}
```

### 4.9 `accept_call_response`

Возможные статусы: `OK` (0), `NoSuchCall` (9), `NotCallParticipant` (10),
`CallAlreadyProceeded` (13), `InternalError` (255).

```json
{
  "type": "accept_call_response",
  "payload": { "status": 0, "call_id": "..." }
}
```

### 4.10 `call_accepted`

Уведомление инициатору о том, что вызываемый принял звонок.

```json
{
  "type": "call_accepted",
  "payload": { "call_id": "..." }
}
```

### 4.11 `reject_call_response`

Возможные статусы: `OK` (0), `NoSuchCall` (9), `NotCallParticipant` (10),
`CallAlreadyProceeded` (13), `InternalError` (255).

```json
{
  "type": "reject_call_response",
  "payload": { "status": 0, "call_id": "..." }
}
```

### 4.12 `call_rejected`

Уведомление инициатору о том, что вызываемый отклонил звонок.

```json
{
  "type": "call_rejected",
  "payload": { "call_id": "..." }
}
```

### 4.13 `end_call_response`

Возможные статусы: `OK` (0), `NoSuchCall` (9), `NotCallParticipant` (10),
`InternalError` (255).

```json
{
  "type": "end_call_response",
  "payload": { "status": 0, "call_id": "..." }
}
```

### 4.14 `call_ended`

Уведомление второй стороне о том, что звонок завершен.

```json
{
  "type": "call_ended",
  "payload": { "call_id": "..." }
}
```

### 4.15 `sdp` (сервер → клиент)

Ретрансляция SDP второй стороне звонка (`status: 0`), либо ответ об ошибке
отправителю (см. [4.5](#45-sdp--обмен-sdp-offeranswer)). Формат payload
совпадает с командой `sdp`, отправленной клиентом, плюс поле `status`.

### 4.16 `ice_candidate` (сервер → клиент)

Ретрансляция ICE-кандидата второй стороне звонка (`status: 0`), либо ответ
об ошибке отправителю (см.
[4.6](#46-ice_candidate--обмен-ice-кандидатами)). Формат payload совпадает
с командой `ice_candidate`, отправленной клиентом, плюс поле `status`.
