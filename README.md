# SigmaMessenger

SigmaMessenger — децентрализованный мессенджер с поддержкой текстовых сообщений, голосовых и видеозвонков (WebRTC). Проект написан на C++17 с использованием Qt 6. Серверная часть управляет авторизацией, друзьями и сигналингом звонков, клиентская — предоставляет QML-интерфейс.

---

## Требования для сборки

### Общие
- **CMake** ≥ 3.28
- **Компилятор** с поддержкой C++17 (GCC 9+, Clang 10+, MSVC 2019+)
- **Git** (для подтягивания зависимостей)

### Зависимости

#### Сервер
- **Qt 6.8+** с компонентами: `Core`, `Network`, `WebSockets`, `Sql`
- **PostgreSQL** (сервер использует драйвер `QPSQL`)
- **libsodium** (для хэширования паролей)
  - Проект также включает предварительно собранные библиотеки в `third-party/libsodium/` для Windows и Linux.

#### Клиент
- **Qt 6.8+** с компонентами: `Gui`, `Qml`, `Quick`, `Core`, `Network`, `WebSockets`, `Multimedia`, `Sql`
- **libdatachannel** (подтягивается автоматически через CMake FetchContent)
- **FFmpeg** (libavcodec, libavutil, libswscale, libswresample) – для обработки видео/аудио в звонках
  - Установите через пакетный менеджер:
    - Ubuntu/Debian: `libavcodec-dev libavutil-dev libswscale-dev libswresample-dev`
    - Windows: используйте готовые сборки или [vcpkg](https://vcpkg.io/).

---

## Сборка

### Linux (Ubuntu/Debian)

1. Установите основные зависимости:
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-declarative-dev qt6-websockets-dev libqt6sql6-psql libsodium-dev libavcodec-dev libavutil-dev libswscale-dev libswresample-dev
```

2. Клонируйте репозиторий:
```bash
git clone https://github.com/elochkaan-git/sigma.git
cd sigma
```

3. Создайте директорию сборки и настройте CMake:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```
*Если Qt установлен нестандартно, укажите `-DCMAKE_PREFIX_PATH=/path/to/qt6`.*

4. Соберите проект:
```bash
cmake --build . --config Release -j$(nproc)
```

Исполняемые файлы:
- Клиент: `build/client/SigmaMessenger`
- Сервер: `build/server/server`

### Windows (Visual Studio)

1. Установите **Visual Studio 2019+** с компонентом "Разработка приложений на C++".
2. Установите **Qt 6.8+** через онлайн-установщик (выберите компоненты для MSVC и QML).
3. Установите **libsodium** (скачайте DLL и LIB, поместите в `third-party/libsodium/lib/windows/`).
4. Установите **FFmpeg** (разместите заголовки и библиотеки в системных путях или задайте переменные окружения).
5. Откройте командную строку разработчика Visual Studio и выполните:
```cmd
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.8.0/msvc2019_64" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Опции CMake

- `-DNO_SERVER=ON` – пропустить сборку сервера (только клиент).
- `-DNO_CLIENT=ON` – пропустить сборку клиента (только сервер).
- `-DPACKAGING=ON` – включить создание пакета (ZIP/TGZ) с версией, извлечённой из Git-тега.

---

## Конфигурация

### Сервер

Для работы сервера требуется два конфигурационных источника:

#### 1. `config.ini` – настройки сети и безопасности
Файл должен лежать в рабочей директории сервера. Пример:

```ini
[network]
host=0.0.0.0
port=5555
flood_limit=1
ban_limit=60
max_msg_size=536870912
max_login_attempts=4
max_oversized_msgs=4
max_msgs_allowed=1000
trust_proxy_server=false

[turn]
secret=supersecret
```

| Секция | Параметр | Описание |
|--------|----------|----------|
| `[network]` | `host` | IP-адрес для прослушивания (по умолчанию `0.0.0.0`) |
| | `port` | Порт WebSocket-сервера (по умолчанию `5555`) |
| | `flood_limit` | Время (в минутах) блокировки за флуд (по умолчанию `5`) |
| | `ban_limit` | Время (в минутах) бана за превышение лимитов (по умолчанию `60`) |
| | `max_msg_size` | Максимальный размер входящего сообщения в байтах (по умолчанию `512 МБ`) |
| | `max_login_attempts` | Допустимое число неудачных попыток входа до бана (по умолчанию `4`) |
| | `max_oversized_msgs` | Допустимое число слишком больших сообщений до бана (по умолчанию `4`) |
| | `max_msgs_allowed` | Максимальное количество сообщений в единицу времени до флуд-блокировки (по умолчанию `1000`) |
| | `trust_proxy_server` | Использовать заголовок `X-Real-IP` (если сервер за прокси) – `false` по умолчанию |
| `[turn]` | `secret` | Секретный ключ для генерации временных TURN-учётных данных (обязателен для звонков) |

#### 2. Переменные окружения – подключение к базе данных

Сервер использует PostgreSQL. Переменные задаются перед запуском (см. `.env.example`):

```bash
export DB_HOST=localhost
export DB_USER=user
export DB_PASSWORD=password
export DB_NAME=db
```

Или через `.env`-файл при запуске в Docker (см. ниже).

#### 3. Настройка TURN-сервера (для звонков)

Для корректной работы аудио/видеозвонков через WebRTC необходим TURN-сервер. В репозитории приведён пример конфигурации turnserver.conf.example. Основные параметры:

    listening-port и tls-listening-port – порты для UDP/TLS-соединений.

    external-ip – публичный IP-адрес сервера

    use-auth-secret и static-auth-secret – включают аутентификацию по общему секрету (должен совпадать с secret в config.ini сервера).

    min-port/max-port – диапазон портов для ретрансляции медиатрафика.

После установки Coturn скопируйте этот файл, отредактируйте под свои нужды и запустите TURN-сервер через docker

```sh
sudo make up # сервис поднимается вместе с остальными
# или запустите точечно: sudo docker compose up coturn
```

или напрямую:
```bash
turnserver -c /path/to/turnserver.conf
```
Убедитесь, что в config.ini сервера Sigma в секции [turn] указан тот же secret, что и в static-auth-secret TURN-сервера. Тогда клиенты, запросившие get_turn_credentials, получат временные логин/пароль, валидные для доступа к TURN.

---

## Запуск

### На хосте

1. Убедитесь, что PostgreSQL запущен и база данных создана.
2. Задайте переменные окружения для БД.
3. Запустите сервер из директории `build`:
```bash
DB_HOST=localhost DB_USER=user DB_PASSWORD=password DB_NAME=db ./server/server
```

Для клиента просто запустите `./SigmaMessenger`.

### В Docker

Проект включает `docker-compose.yml` и `Dockerfile` для развёртывания сервера в контейнере.

1. В корневой директории выполните:
```bash
sudo make up
```
Эта команда соберёт базовый образ, затем образ сервера и запустит контейнеры.

2. Альтернативно, используйте `docker-compose` напрямую (предварительно соберите образ `base`):
```bash
docker-compose up -d
```

Переменные окружения для БД можно переопределить в `.env`-файле (скопируйте `.env.example` в `.env` и отредактируйте).

---

## Структура проекта

```
sigma/
├── CMakeLists.txt                    # корневой CMake
├── README.md
├── PROTOCOL.md                       # полное описание протокола WebSocket
├── config.ini.example                # пример конфигурации сервера
├── .env.example                      # пример переменных БД
├── common/                           # общий код (структуры данных, утилиты)
│   ├── inc/                          # заголовки
│   └── src/                          # исходники
├── server/
│   ├── CMakeLists.txt
│   ├── inc/                          # заголовки сервера
│   └── src/                          # исходники сервера (обработчики команд, БД)
├── client/
│   ├── CMakeLists.txt
│   ├── include/                      # заголовки клиента
│   ├── src/                          # исходники клиента (сеть, звонки)
│   ├── qml/                          # QML-компоненты интерфейса
│   └── assets/                       # иконки и ресурсы
├── third-party/
│   └── libsodium/                    # статическая библиотека libsodium (для Windows/Linux)
└── docker/                           # файлы для контейнеризации
```

---

## Протокол

Полное описание форматов сообщений, кодов статусов и последовательности команд для звонков находится в [PROTOCOL.md](PROTOCOL.md). Сервер принимает бинарные WebSocket-сообщения с JSON-содержимым, сжатие не используется.

---

## Дополнительная информация

- **WebRTC-звонки** реализованы через libdatachannel. Сигналинг (SDP, ICE) проходит через сервер, медиатрафик – напрямую между клиентами (или через TURN при необходимости).
- **Аватарки** передаются в base64, размер декодированного изображения ограничен 1 МБ.
- **Логирование** сервера и клиента настроено на вывод в консоль с контекстом (`QT_MESSAGELOGCONTEXT`).
- Для сборки клиента на Linux требуется установить **Qt 6.8+** с модулем `qt6-qml-modules-qtquick-templates2` (для QML-стилей).
