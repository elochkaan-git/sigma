#pragma once
#include <QObject>
#include <QString>
#include <QDebug>
#include <QVariantList>
#include <QTimer>

class ClientController : public QObject
{
    Q_OBJECT
    // Свойство для отслеживания статуса подключения
    Q_PROPERTY(QVariantList serversList READ getServersList NOTIFY serversStatusChanged)

public:
    explicit ClientController(QObject *parent = nullptr);

    bool isConnected() const { return m_isConnected; }
    const QVariantList& getServersList() const { return serversList; } // Геттер для списка серверов

    Q_INVOKABLE QVariantList loadServersFromCsv(const QString &csvPath = QString());
    Q_INVOKABLE void addServer(const QString &name, const QString &url);
    Q_INVOKABLE void removeServer(int index);
    Q_INVOKABLE void updateServer(int index, const QString &name, const QString &url);
    Q_INVOKABLE void setSelectedServer(const QVariant& server);
    Q_INVOKABLE void registerUser(const QString &login, const QString &password);
    Q_INVOKABLE void loginUser(const QString &login, const QString &password);

signals:
    void serversStatusChanged(); // Сигнал для уведомления об изменении статуса серверов
    void serversListChanged(); // Сигнал для уведомления об изменении списка серверов
    void pingServer(const QString &url);
    void registerRequest(const QString &login, const QString &password);
    void loginRequest(const QString &login, const QString &password);
    void serverSelected(const QString &url);

private slots:
    void updateCSV();
    void pingAllServers();


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

    QTimer *pingTimer = nullptr; // Таймер для периодического пинга серверов
};
