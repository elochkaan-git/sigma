# SigmaMessenger

SigmaMessenger — это приложение на базе C++ и Qt 6 (QML/Quick).

## Требования для сборки (Prerequisites)

Перед началом сборки убедитесь, что в вашей системе установлены следующие компоненты:

1. **CMake** версии **3.28** или выше.
2. **Компилятор** с поддержкой стандарта **C++17** (GCC / Clang на Linux, MSVC 2019+ на Windows).
3. **Qt 6** [версии **6.8** или выше] с компонентами Gui, Qml, Quick (для сборки клиента) и Core, Network, WebSockets, Sql (для сборки сервера).

---

## Сборка на Linux

### 1. Установка зависимостей (на примере Ubuntu/Debian)

> [!warning] Обратите внимание
> Названия пакетов Qt6 могут отличаться в зависимости от вашего дистрибутива Linux

#### Клиент
Установите необходимые инструменты сборки и пакеты Qt 6:

``` bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev qt6-qml-modules-qtquick-templates2
# Альтернативный вариант: сборка из исходников (https://doc.qt.io/qt-6/linux-building.html)
```

#### Сервер

```sh
apt-get update
apt-get install -y --no-install-recommends build-essential cmake ninja-build pkg-config ca-certificates qt6-base-dev qt6-websockets-dev libqt6sql6-psql
```

### 2. Компиляция проекта

Выполните следующие команды в корневой директории проекта:

``` bash
# 1. Копирование проекта и переход в него
git clone https://github.com/elochkaan-git/sigma.git && cd sigma

# 2. Создание и переход в директорию сборки
mkdir build && cd build

# 3. Конфигурация проекта
# Если Qt установлен в нестандартную директорию, укажите -DCMAKE_PREFIX_PATH=/путь/к/qt6
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Сборка приложения
cmake --build . --config Release
```

Исполняемый файл клиента `SigmaMessenger` будет находиться непосредственно в папке `build`, исполняемый файл сервера будет находиться в `build/server`.

## Сборка на Windows

### 1. Подготовка окружения

1. Установите **Visual Studio** (с инструментами разработки C++) или **MinGW**.
2. Установите **Qt 6.8+** через Qt Online Installer. При установке обязательно выберите компоненты `Qt Quick` и `Qt QML`.
3. Убедитесь, что пути к `cmake.exe` и вашей установке Qt добавлены в системную переменную `PATH`

### 2. Компиляция проекта

Откройте командную строку (cmd) или PowerShell в корневой директории проекта и выполните:

``` cmd
:: 1. Создание и переход в директорию сборки
mkdir build
cd build

:: 2. Конфигурация проекта
:: Замените "C:/Qt/6.8.0/msvc2019_64" на ваш реальный путь к установленной версии Qt
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.8.0/msvc2019_64" -DCMAKE_BUILD_TYPE=Release

:: 3. Сборка приложения
cmake --build . --config Release
```

## Структура проекта

- `main.cpp` — Точка входа в приложение.
- `qml/` — Исходный код интерфейса на QML (Main, Login, Register и др.).
- `assets/` — Графические ресурсы приложения (логотипы, иконки).

---

## Запуск сервера

Запустить сервер можно двумя способами: в контейнере и непосредственно на хосте

### Запуск в контейнере

Запуск всего приложения производится командой в корневой директории проекта

```sh
sudo make up
```

Данная команда сначала соберет базовый образ системы для сборки проекта, а уже потом непосредственно проект

> [!warning] Предупреждение
> Если вы собираетесь запускать сервер через непосредственно docker-compose, то сначала нужно собрать сервис base, а уже потом server

### Запуск на хосте

Запуск производится через исполняемый файл `/path/to/project/sigma/build/server/server`. Но для корректного запуска должны быть определены переменные окружения (см. [.env.example](https://github.com/elochkaan-git/sigma/blob/main/.env.example)). Дальше приведен пример запуска сервера с заданными переменными окружения

```sh
DB_HOST=localhost DB_USER=user DB_PASSWORD=password DB_NAME=db bash -c "./server/server"
```