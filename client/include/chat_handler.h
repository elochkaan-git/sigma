#pragma once
#include <QObject>
#include <QVariantList>
#include <QtSql/QSqlDatabase>
#include <QQueue>
#include <QtQml/qqmlregistration.h>
#include "responses.h"

struct ChatMessage {
    int messageId;
    int senderId;
    int receiverId;
    QString content;
    QString timestamp;
    bool isOutgoing;
};

struct PendingMessage {
    unsigned int receiverId;
    QString content;
};

class ChatHandler : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantList chatsList READ chatsList NOTIFY chatsListChanged)
    Q_PROPERTY(QVariantList currentChatMessages READ currentChatMessages NOTIFY currentChatMessagesChanged)

public:
    explicit ChatHandler(QObject *parent = nullptr);
    ~ChatHandler();

    // Инициализация БД при успешном авторизации пользователя
    void initDatabaseForUser(unsigned int currentUserId, const QString &serverIdentifier);

    // Вызывается из QML
    Q_INVOKABLE void loadChatWithUser(int peerUserId);
    Q_INVOKABLE void deleteChatHistory(int peerUserId);
    Q_INVOKABLE void saveAndSendMessage(unsigned int receiverId, const QString &content);

    // Обработчики сетевых ответов из ClientController
    void handleNewMessage(const wire::NewMessageResponse& r);
    void handleSendMessage(const wire::SendMessageResponse& r);

    QVariantList chatsList() const { return m_chatsList; }
    QVariantList currentChatMessages() const { return m_currentChatMessages; }

signals:
    void chatsListChanged();
    void currentChatMessagesChanged();
    void chatLoadedSuccessfully();
    void sendMessageRequested(unsigned int recieverId, const QString& content);
    void showErrorToast(const QString &message);

public slots:
    void updateChatsList(); // Обновление списка активных чатов из БД

private:
    void saveMessageToDb(int senderId, int receiverId, int peerId, const QString &content, bool isOutgoing);

    QSqlDatabase m_db;
    unsigned int m_currentUserId = 0;
    int m_activePeerId = -1; // Чат, открытый в данный момент в UI

    QVariantList m_chatsList;
    QVariantList m_currentChatMessages;
    QQueue<PendingMessage> m_pendingMessages;
};
