#include "client_controller.h"
#include "commands.h"
#include "transport.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QTimer>
#include <QDebug>
#include <QImage>
#include <QBuffer>
#include <QtMultimedia>

namespace {
QString resolveServersCsvPath(const QString &requestedPath)
{
    QStringList candidates;

    const QString trimmedPath = requestedPath.trimmed();
    if (!trimmedPath.isEmpty()) {
        candidates << trimmedPath;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();

    candidates << appDir + "/servers.csv";
    candidates << appDir + "/assets/servers.csv";
    candidates << appDir + "/Main/assets/servers.csv";
    candidates << currentDir + "/servers.csv";
    candidates << currentDir + "/assets/servers.csv";
    candidates << currentDir + "/Main/assets/servers.csv";
    candidates << QStringLiteral(":/assets/servers.csv");
    candidates << QStringLiteral(":/Main/assets/servers.csv");
    candidates << QStringLiteral("qrc:/assets/servers.csv");
    candidates << QStringLiteral("qrc:/Main/assets/servers.csv");

    for (const QString &candidate : candidates) {
        if (candidate.isEmpty()) {
            continue;
        }

        if (candidate.startsWith(":/", Qt::CaseInsensitive) || candidate.startsWith("qrc:/", Qt::CaseInsensitive)) {
            if (QFile::exists(candidate)) {
                return candidate;
            }
            continue;
        }

        const QFileInfo fileInfo(candidate);
        if (fileInfo.exists() && fileInfo.isFile()) {
            return fileInfo.absoluteFilePath();
        }
    }

    return trimmedPath;
}
} // namespace

ClientController::ClientController(QObject *parent)
    : QObject(parent)
    , m_transport(new Transport(this))
    , call_manager_(m_transport, this)
    , m_isConnected(false)
{
    // updateCSV вызывается только при добавлении/удалении/редактировании
    connect(this, &ClientController::serversListChanged, this, &ClientController::updateCSV);
    connect(this, &ClientController::loadFromCSVEnded, this, &ClientController::onLoadFromCSVEnded);

    pingTimer = new QTimer(this);
    connect(pingTimer, &QTimer::timeout, this, &ClientController::pingAllServers);
    pingTimer->start(5000); // Опрос раз в 5 секунд

    aliveTimer = new QTimer(this);
    connect(aliveTimer, &QTimer::timeout, this, &ClientController::pingConnectedServer);
    aliveTimer->setInterval(5000);

    // Transport layer creation
    // m_transport = new Transport(this);
    connect(m_transport, &Transport::responseReady, 
            this, &ClientController::handleTransportResponse);
    
    connect(&chat_handler_, &ChatHandler::sendMessageRequested, 
            this, &ClientController::sendMessagetoUser);
    
    connect(&auth_handler_, &AuthHandler::loginSuccess,
            this, &ClientController::onLoginSuccess);

    connect(&auth_handler_, &AuthHandler::friendsChanged, 
            &chat_handler_, &ChatHandler::updateChatsList);
    
    call_manager_.setDevices({
        QMediaDevices::defaultAudioInput(),
        QMediaDevices::defaultVideoInput()
    });
    //TODO: make code check
    call_manager_.initialize();
    m_remoteVideoProvider = new VideoImageProvider(this);
    m_localVideoProvider = new VideoImageProvider(this);
    m_audioOutput = new AudioOutput(this);
    if (!m_audioOutput->init(48000, 1)) {
        qWarning() << "Failed to init audio output";
    }

    connect(&call_manager_, &CallManager::remoteVideoFrameReady,
            this, [this](const QImage& img) {
                qDebug() << "remoteVideoFrameReady, image size:" << img.size();
                m_remoteVideoProvider->updateFrame(img);
                ++m_remoteVideoVersion;
                emit remoteVideoVersionChanged();
            });

    connect(&call_manager_, &CallManager::localVideoFrameReady,
            this, [this](const QImage& img) {
                qDebug() << "localVideoFrameReady, image size:" << img.size();
                m_localVideoProvider->updateFrame(img);
                ++m_localVideoVersion;
                emit localVideoVersionChanged();
            });

    connect(&call_manager_, &CallManager::decodedAudioReady,
            this, [this](const QByteArray& pcmData, int sampleRate, int channels) {
                qDebug() << "decodedAudioReady, PCM size:" << pcmData.size()
                        << "sampleRate:" << sampleRate << "channels:" << channels;
                m_audioOutput->writePCM(pcmData);
            });

    connect(&call_manager_, &CallManager::callClosed, m_audioOutput, &AudioOutput::reset);
    connect(&call_manager_, &CallManager::callFailed, m_audioOutput, &AudioOutput::reset);
}

void ClientController::setAvatarProvider(AvatarImageProvider *avatarProvider)
{
    m_avatarProvider = avatarProvider;
    auth_handler_.setAvatarProvider(avatarProvider);
}

VideoImageProvider* ClientController::remoteVideoProvider()
{
    return this->m_remoteVideoProvider;
}

VideoImageProvider* ClientController::localVideoProvider()
{
    return this->m_localVideoProvider;
}


QVariantList ClientController::loadServersFromCsv(const QString &csvPath)
{
    const QString path = resolveServersCsvPath(csvPath);
    pathToServersCsv = path; // Сохраняем путь к CSV-файлу для последующего обновления
    qDebug() << "Загрузка списка серверов из CSV-файла:" << path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть CSV-файл серверов:" << path;
        return {};
    }

    QTextStream in(&file);
    QVariantList servers;

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        const QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() < 2)
            continue;

        const QString name = parts.at(0).trimmed();
        const QString url = parts.at(1).trimmed();

        QVariantMap serverEntry;
        serverEntry.insert("serverName", name);
        serverEntry.insert("serverUrl", url);
        serverEntry.insert("isOnline", false); // Изначально считаем сервер оффлайн
        serverEntry.insert("usersCount", 0);    // Изначально считаем,
        serverEntry.insert("onlineCount", 0);   // что пользователей нет
        servers.append(serverEntry);
    }

    serversList = servers; // Сохраняем список серверов в свойство класса
    emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился
    emit loadFromCSVEnded();

    return servers;
}

Q_INVOKABLE void ClientController::addServer(const QString &name, const QString &url)
{
    QVariantMap serverEntry;
    serverEntry.insert("serverName", name);
    serverEntry.insert("serverUrl", url);
    serverEntry.insert("isOnline", false); 
    serverEntry.insert("usersCount", 0);    
    serverEntry.insert("onlineCount", 0);     
    serversList.append(serverEntry);

    emit serversListChanged(); // Уведомляем об изменении списка серверов
    emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился

    pingAllServers(); // Пингуем сразу после добавления нового сервера
}

Q_INVOKABLE void ClientController::removeServer(int index)
{
    if (index >= 0 && index < serversList.size()) {
        serversList.removeAt(index);
        emit serversListChanged(); // Уведомляем об изменении списка серверов
        emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился
    }
}

Q_INVOKABLE void ClientController::updateServer(int index, const QString &name, const QString &url)
{
    if (index >= 0 && index < serversList.size()) {
        QVariantMap serverEntry;
        serverEntry.insert("serverName", name);
        serverEntry.insert("serverUrl", url);
        serversList.replace(index, serverEntry);
        emit serversListChanged(); // Уведомляем об изменении списка серверов
        emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился
    }
}

Q_INVOKABLE void ClientController::setSelectedServer(const QVariant &server)
{
    m_transport->disconnectFromHost();
    selectedServer = server; 
    QString serverUrl = selectedServer.toMap()["serverUrl"].toString();
    qDebug() << "Selected server URL:" << serverUrl;
    m_transport->connectToHost(serverUrl);
    pingTimer->stop();
    aliveTimer->start();
    connectedToURL = serverUrl;
}

Q_INVOKABLE void ClientController::disconnectFromServer(){
    m_transport->disconnectFromHost();
    qDebug() << "Disconected from:" << selectedServer.toMap().value("serverUrl").toString();
    selectedServer = {};
    pingTimer->start();
    aliveTimer->stop();
    emit serverDisconected();
}

Q_INVOKABLE void ClientController::registerUser(const QString &login, const QString &password) {
    m_transport->sendCommand(wire::RegisterUser{login, password});
}

Q_INVOKABLE void ClientController::loginUser(const QString &login, const QString &password) {
    m_transport->sendCommand(wire::LoginUser{login, password});
}

Q_INVOKABLE void ClientController::sendFriendRequest(unsigned int userId){
    m_transport->sendCommand(wire::SendFriendRequest{userId});
}

Q_INVOKABLE void ClientController::getAllFriendsInfo()
{
    m_transport->sendCommand(wire::GetFriends{});
    m_transport->sendCommand(wire::GetFriendRequests{});
    m_transport->sendCommand(wire::GetSentFriendRequests{});
}

Q_INVOKABLE void ClientController::acceptFriendRequest(unsigned int userId)
{
    m_transport->sendCommand(wire::AcceptFriendRequest{userId});
}

Q_INVOKABLE void ClientController::rejectFriendRequest(unsigned int userId)
{
    m_transport->sendCommand(wire::RejectFriendRequest{userId});
}

void ClientController::updateCSV(){
    if(pathToServersCsv.isEmpty()){
        qWarning() << "Путь к CSV-файлу серверов не установлен.";
        return;
    }

    QFile file(pathToServersCsv);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Не удалось открыть CSV-файл для записи:" << pathToServersCsv;
        return;
    }
    QTextStream out(&file);

    out << "# serverName,serverUrl\n"; // Заголовок CSV-файла
    for (const QVariant &serverVar : serversList) {
        const QVariantMap serverEntry = serverVar.toMap();
        const QString name = serverEntry.value("serverName").toString();
        const QString url = serverEntry.value("serverUrl").toString();
        out << name << "," << url << "\n";
    }
    file.close();
    qDebug() << "Список серверов успешно обновлен в CSV-файле:" << pathToServersCsv;

}

void ClientController::pingAllServers()
{
    qDebug() << "Запуск параллельного пинга всех серверов...";

    for (const QVariant &serverVar : serversList) {
        const QVariantMap serverEntry = serverVar.toMap();
        QString url = serverEntry.value("serverUrl").toString();

        // if (!url.startsWith("ws://") && !url.startsWith("wss://")) {
        //     // Если нет, принудительно добавляем ws:// в начало
        //     url = QString("ws://") + url;
        // }
        
        // Создаем отдельный транспорт для этого конкретного сервера
        Transport* pingerTransport = new Transport(this); 

        connect(pingerTransport, &Transport::responseReady, this, [this, url, pingerTransport](const Response& response) {
            
            // Проверяем, что пришел именно ответ на GetServerStats
            if (std::holds_alternative<wire::GetServerStatsResponse>(response)) {
                auto stats = std::get<wire::GetServerStatsResponse>(response);

                qInfo() << "Server stats: online=" << stats.online << ", total=" << stats.total;
                // Обновляем статус сервера (вызываем твой существующий слот)
                this->updateServerStatus(url, true, stats.total, stats.online);
            }

            // Важно: удаляем этот временный транспорт после получения ответа
            pingerTransport->disconnectFromHost();
            pingerTransport->deleteLater();
        });

        connect(pingerTransport, &Transport::connected, this, [pingerTransport]() {
            pingerTransport->sendCommand(wire::GetServerStats{});
        });

        QTimer::singleShot(5000, pingerTransport, [pingerTransport, this, url]() {
            if (pingerTransport) {
                qDebug() << "Сервер" << url << "не ответил по таймауту.";
                this->updateServerStatus(url, false, -1, -1);
                pingerTransport->disconnectFromHost();
                pingerTransport->deleteLater();
            }
        });
        // 4. Запускаем процесс для конкретного URL
        pingerTransport->connectToHost(url);
    }
}

void ClientController::handleTransportResponse(const Response &response)
{
    std::visit(overloaded{
        // 1. Общие системные сообщения
        [this](const wire::Error& r) { handleError(r); },

        // 2. Модуль авторизации и профиля
        [this](const wire::RegisterUserResponse& r)          { auth_handler_.handleRegister(r); },
        [this](const wire::LoginUserResponse& r)             { auth_handler_.handleLogin(r); },

        [this](const wire::GetFriendsResponse& r)            { auth_handler_.handleGetFriends(r); },
        [this](const wire::GetFriendRequestsResponse& r)     { auth_handler_.handleGetFriendRequests(r); },
        [this](const wire::GetSentFriendRequestsResponse& r) { auth_handler_.handleGetSentFriendRequests(r); },

        [this](const wire::SendFriendRequestResponse& r)     { auth_handler_.handleSendFriendRequest(r); },
        [this](const wire::AcceptFriendRequestResponse& r)   { auth_handler_.handleAcceptFriendRequest(r); },
        [this](const wire::RejectFriendRequestResponse& r)   { auth_handler_.handleRejectFriendRequest(r); },
        [this](const wire::RemoveFriendResponse& r)          { auth_handler_.handleRemoveFriend(r); },
        [this](const wire::GetOnlineUsersResponse& r)        { auth_handler_.handleGetOnlineUsers(r);},
        [this](const wire::SetAvatarResponse& r)             { auth_handler_.handleSetAvatar(r);},
        
        // 3. Модуль обмена сообщениями (Чаты)
        [this](const wire::SendMessageResponse& r) { chat_handler_.handleSendMessage(r); },
        [this](const wire::NewMessageResponse& r)  { chat_handler_.handleNewMessage(r); },        

        // // 4. Модуль звонков (VoIP / WebRTC)
        [this](const wire::StartCallResponse& r)   { call_manager_.handleStartCallResponse(r); },
        [this](const wire::IncomingCallResponse& r){ call_manager_.handleIncomingCall(r); },
        [this](const wire::AcceptCallResponse& r)  { call_manager_.handleAcceptCallResponse(r); },
        [this](const wire::CallAcceptedResponse& r){ call_manager_.handleCallAccepted(r); },
        [this](const wire::RejectCallResponse& r)  { call_manager_.handleRejectCallResponse(r); },
        [this](const wire::CallRejectedResponse& r){ call_manager_.handleCallRejected(r); },
        // [this](const wire::EndCallResponse& r)     { call_manager_.handleEndCall(r); },
        [this](const wire::CallEndedResponse& r)   { call_manager_.handleCallEnded(r); },
        [this](const wire::SdpResponse& r)         { call_manager_.handleSdp(r); },
        [this](const wire::IceCandidateResponse& r) { call_manager_.handleIceCandidate(r); },
        [this](const wire::GetTurnCredentialsResponse& r) {
            // Извлекаем IP из connectedToURL (убираем протокол и порт)
            QString ip = connectedToURL;
            ip.remove("ws://", Qt::CaseInsensitive);
            ip.remove("wss://", Qt::CaseInsensitive);
            // Обрезаем порт, если есть
            if (ip.contains(':')) {
                ip = ip.section(':', 0, 0);
            }
            call_manager_.handleGetTurnCredentials(ip, r);
        },

        // Временная заглушка для ВСЕХ остальных типов
        [](const auto&) {
        //    qDebug() << "Unhandled response type index:" << unhandled_response.status;
        }

        // // 5. Системная статистика / Утилиты
        // [this](const wire::GetServerStatsResponse& r)     { handleGetServerStats(r); },
    }, response);
}

void ClientController::onLoadFromCSVEnded()
{
    pingAllServers();
}

void ClientController::pingConnectedServer()
{
    m_transport->sendCommand(wire::GetServerStats{});
}

void ClientController::onLoginSuccess(unsigned int userId)
{
    qDebug() << "Login succeeded!";
    chat_handler_.initDatabaseForUser(userId, connectedToURL);
    this->getAllFriendsInfo();
    m_transport->sendCommand(wire::GetTurnCredentials{});
}

void ClientController::updateFriendsInfo()
{
    m_transport->sendCommand(wire::GetFriends{});
}

void ClientController::updateIncomingFriendsRequests()
{
    m_transport->sendCommand(wire::GetFriendRequests{});
}

void ClientController::updateOutcomingFriendsRequests()
{
    m_transport->sendCommand(wire::GetSentFriendRequests{});
}

Q_INVOKABLE void ClientController::deleteFriend(unsigned int userId)
{
    m_transport->sendCommand(wire::RemoveFriend{userId});
}

Q_INVOKABLE void ClientController::updateOnlineUsers()
{
    m_transport->sendCommand(wire::GetOnlineUsers{});
}

Q_INVOKABLE void ClientController::setAvatarRequest(const QString &avatarFilePath)
{
    QUrl url(avatarFilePath);
    QString localPath = url.isLocalFile() ? url.toLocalFile() : avatarFilePath;

    // 1. Загружаем изображение через QImage
    QImage img(localPath);
    if (img.isNull()) {
        qWarning() << "Не удалось загрузить или распознать изображение:" << localPath;
        return;
    }

    // 2. Уменьшаем размер аватарки (например, макс. 256x256), сохраняя пропорции
    QImage scaledImg = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 3. Сохраняем в байтовый массив как PNG
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    scaledImg.save(&buffer, "PNG");

    // 4. Кодируем в Base64 без символов переноса строк
    QString base64Image = QString::fromLatin1(ba.toBase64());

    // Отправляем сжатый и валидный PNG
    m_transport->sendCommand(wire::SetAvatar{base64Image});
}

Q_INVOKABLE void ClientController::startCallRequest(unsigned int userId, bool withVideo)
{
    qDebug() << "Requesting call to user:" << userId << "with video:" << withVideo;
    call_manager_.startCall(userId, withVideo);
}

Q_INVOKABLE void ClientController::acceptCallRequest(){
    call_manager_.acceptCall();
}

Q_INVOKABLE void ClientController::rejectCallRequest()
{
    call_manager_.rejectCall();
}

Q_INVOKABLE void ClientController::endCallRequest()
{
    call_manager_.endCall();
}

void ClientController::sendMessagetoUser(unsigned int userId, const QString &message)
{
    qDebug() << "Sending message to " << userId << ": " << message;
    m_transport->sendCommand(wire::SendMessage{userId, message});
}

void ClientController::handleError(const wire::Error &error)
{
    qInfo() << "Error:" << error.reason;
}

void ClientController::updateServerStatus(const QString &url, bool isOnline, int usersCount, int onlineCount){
    bool modified = false;
    for (int i = 0; i < serversList.size(); ++i) {
        QVariantMap serverEntry = serversList.at(i).toMap();
        if (serverEntry.value("serverUrl").toString() == url) {
            serverEntry["isOnline"] = isOnline;
            serverEntry["usersCount"] = usersCount;   // Общее число пользователей
            serverEntry["onlineCount"] = onlineCount; // Пользователей онлайн

            serversList.replace(i, serverEntry);
            modified = true;
            break;
        }
    }
    if (modified) {
        // Уведомляем QML, что статус обновился (без перезаписи CSV на диске!)
        emit serversStatusChanged(); 
    }
}
