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


### Типы данных

| C++ тип         | JSON тип                          |
|-----------------|------------------------------------|
| `QString`       | `string`                           |
| `unsigned int`  | `number` (целое, ≥ 0)              |
| `OperationStatus` | `number` (целое, код статуса — см. таблицу ниже) |
| `User`          | `object` (`{ "user_id": number, "login": string }`) |
| `std::vector<User>` (опционально) | `array` объектов `User`; если данных нет — пустой массив `[]` |

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
      { "user_id": 2, "login": "alice" },
      { "user_id": 3, "login": "bob" }
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
      { "user_id": 4, "login": "carol" }
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
      { "user_id": 5, "login": "dave" }
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

### 3.13 `error`

Возвращают ошибку, непредусмотренную в OperationStatus, в виде текста.

```json
{
  "type": "error",
  "payload": {
    "reason": "string"
  }
}
```