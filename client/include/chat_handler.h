#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QVariantList>
#include <QVariantMap>
#include "wire_command_types.h"
#include "command_types.h"
#include "commands.h"
#include "responses.h"
#include "structures.h"

class ChatHandler : public QObject {
    Q_OBJECT
    QML_ELEMENT
    // Позволяет QML читать список чатов через clientController.chatsList
    Q_PROPERTY(QVariantList chatsList READ chatsList NOTIFY chatsListChanged)

public:
    explicit ChatHandler(QObject *parent = nullptr);

    // Метод (INVOKABLE), который вызывается при клике на чат в QML
    Q_INVOKABLE void loadChatWithUser(int userId);

    // Методы для обработки сетевых ответов
    void handleSendMessage(const wire::RegisterUserResponse& r);
    void handleNewMessage(const wire::NewMessageResponse& r);

    // Геттер для свойства chatsList
    QVariantList chatsList() const;
    

signals:
    void chatsListChanged();
    void chatLoadedSuccessfully(); // Сигнал, что история чата загружена

private:
    QVariantList m_chatsList;
};
