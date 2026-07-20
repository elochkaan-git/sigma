#include "chat_handler.h"
#include <QDebug>

ChatHandler::ChatHandler(QObject *parent) : QObject(parent) {
    // Наполняем тестовыми данными для проверки верстки кнопок:
    for(int i = 0; i< 5; i++){
        QVariantMap chat1;
        chat1["userName"] = "Иван Иванов";
        chat1["userId"] = 101;

        QVariantMap chat2;
        chat2["userName"] = "Алексей Смирнов";
        chat2["userId"] = 102;

        QVariantMap chat3;
        chat3["userName"] = "Мария Петрова";
        chat3["userId"] = 103;

        m_chatsList.append(chat1);
        m_chatsList.append(chat2);
        m_chatsList.append(chat3);
    }
}

QVariantList ChatHandler::chatsList() const {
    return m_chatsList;
}

void ChatHandler::loadChatWithUser(int userId) {
    // Этот метод вызывается при клике на кнопку чата
    qDebug() << "C++: Запрос на загрузку чата для пользователя с ID:" << userId;
    
    // Тут в будущем пойдет сетевой запрос:
    // connection->send(wire::GetChatHistoryRequest{userId});
}

void ChatHandler::handleSendMessage(const wire::RegisterUserResponse& r) {
    // Логика обработки отправленного сообщения
}

void ChatHandler::handleNewMessage(const wire::NewMessageResponse& r) {
    // Логика обработки входящего сообщения
}
