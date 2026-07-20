#pragma once
#include <QObject>
#include <QString>
#include <QDebug>
#include <QVariantList>
#include <QTimer>
#include "transport.h"
#include "auth_handler.h"
#include "call_handler.h"
#include "chat_handler.h"

class ClientController : public QObject
{
    Q_OBJECT
    // Свойство для отслеживания статуса подключения
    Q_PROPERTY(QVariantList serversList READ getServersList NOTIFY serversStatusChanged)
    Q_PROPERTY(AuthHandler* authHandler READ authHandler CONSTANT)
    Q_PROPERTY(ChatHandler* chatHandler READ chatHandler CONSTANT)
    Q_PROPERTY(CallHandler* callHandler READ callHandler CONSTANT)

public:
    explicit ClientController(QObject *parent = nullptr);

    bool isConnected() const { return m_isConnected; }
    const QVariantList& getServersList() const { return serversList; } // Геттер для списка серверов
    AuthHandler* authHandler() { return &auth_handler_; }
    ChatHandler* chatHandler() { return &chat_handler_; }
    CallHandler* callHandler() { return &call_handler_; }

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

signals:
    void serversStatusChanged(); // Сигнал для уведомления об изменении статуса серверов
    void serversListChanged(); // Сигнал для уведомления об изменении списка серверов
    void serverSelected(const QString &url);
    void serverDisconected();

private slots:
    void updateCSV();
    void pingAllServers();
    void handleTransportResponse(const Response& response);
    
    


public slots:
    void updateServerStatus(const QString &url, bool isOnline, int usersCount, int onlineCount);
    void showErrorMessage(const QString &message) {
        qWarning() << "Error:" << message;
    }
    
    
private:
    QString pathToServersCsv;
    QVariantList serversList; 
    bool m_isConnected;
    QVariant selectedServer; // Свойство для хранения выбранного сервера
    QString connectedToURL;

    QTimer *pingTimer = nullptr; // Таймер для периодического пинга серверов
    Transport* m_transport = nullptr;

    AuthHandler auth_handler_;
    ChatHandler chat_handler_;
    CallHandler call_handler_;

    void handleError(const wire::Error& error);

};
