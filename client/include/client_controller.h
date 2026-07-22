#pragma once
#include <QObject>
#include <QString>
#include <QDebug>
#include <QVariantList>
#include <QTimer>
#include <QUuid>
#include "transport.h"
#include "auth_handler.h"
#include "call_manager.h"
#include "chat_handler.h"
#include "avatar_image_provider.h"

class ClientController : public QObject
{
    Q_OBJECT
    // Свойство для отслеживания статуса подключения
    
    Q_PROPERTY(QVariantList serversList READ getServersList NOTIFY serversStatusChanged)
    Q_PROPERTY(AuthHandler* authHandler READ authHandler CONSTANT)
    Q_PROPERTY(ChatHandler* chatHandler READ chatHandler CONSTANT)
    Q_PROPERTY(CallManager* callManager READ callManager CONSTANT)

public:
    explicit ClientController(QObject *parent = nullptr);

    bool isConnected() const { return m_isConnected; }
    const QVariantList& getServersList() const { return serversList; } // Геттер для списка серверов
    AuthHandler* authHandler() { return &auth_handler_; }
    ChatHandler* chatHandler() { return &chat_handler_; }
    CallManager* callManager() { return &call_manager_; }
    void setAvatarProvider(AvatarImageProvider* avatarProvider);

    Q_INVOKABLE QVariantList loadServersFromCsv(const QString &csvPath = QString());
    Q_INVOKABLE void addServer(const QString &name, const QString &url);
    Q_INVOKABLE void removeServer(int index);
    Q_INVOKABLE void updateServer(int index, const QString &name, const QString &url);
    Q_INVOKABLE void setSelectedServer(const QVariant& server);
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void registerUser(const QString &login, const QString &password);
    Q_INVOKABLE void loginUser(const QString &login, const QString &password);
    Q_INVOKABLE void sendFriendRequest(unsigned int userId);
    Q_INVOKABLE void getAllFriendsInfo();
    Q_INVOKABLE void acceptFriendRequest(unsigned int userId);
    Q_INVOKABLE void rejectFriendRequest(unsigned int userId);
    Q_INVOKABLE void updateFriendsInfo();
    Q_INVOKABLE void updateIncomingFriendsRequests();
    Q_INVOKABLE void updateOutcomingFriendsRequests();
    Q_INVOKABLE void deleteFriend(unsigned int userId);
    Q_INVOKABLE void updateOnlineUsers();
    Q_INVOKABLE void setAvatarRequest(const QString& avatarFilePath);
    Q_INVOKABLE void startCallRequest(unsigned int userId, bool withVideo);
    Q_INVOKABLE void acceptCallRequest();
    Q_INVOKABLE void rejectCallRequest();
    Q_INVOKABLE void endCallRequest();


signals:
    void serversStatusChanged(); // Сигнал для уведомления об изменении статуса серверов
    void serversListChanged(); // Сигнал для уведомления об изменении списка серверов
    void serverSelected(const QString &url);
    void serverDisconected();
    void loadFromCSVEnded();

private slots:
    void updateCSV();
    void pingAllServers();
    void handleTransportResponse(const Response& response);
    void onLoadFromCSVEnded();
    
    
public slots:
    void updateServerStatus(const QString &url, bool isOnline, int usersCount, int onlineCount);
    void showErrorMessage(const QString &message) {
        qWarning() << "Error:" << message;
    }
    void onLoginSuccess(unsigned int userId);
    void sendMessagetoUser(unsigned int userId, const QString& message);
    
    
private:
    QString pathToServersCsv;
    QVariantList serversList; 
    
    QVariant selectedServer; // Свойство для хранения выбранного сервера
    QString connectedToURL;

    QTimer *pingTimer = nullptr; // Таймер для периодического пинга серверов
    Transport* m_transport = nullptr;
    AvatarImageProvider* m_avatarProvider = nullptr;

    AuthHandler auth_handler_;
    ChatHandler chat_handler_;
    CallManager call_manager_;
    bool m_isConnected;

    void handleError(const wire::Error& error);

};
