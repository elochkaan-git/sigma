#include "chat_handler.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <qcryptographichash.h>

ChatHandler::ChatHandler(QObject *parent) : QObject(parent) {}

ChatHandler::~ChatHandler() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void ChatHandler::initDatabaseForUser(unsigned int currentUserId, const QString &serverIdentifier) {
    m_currentUserId = currentUserId;

    // Закрываем предыдущую БД, если была открыта
    if (m_db.isOpen()) {
        QString connectionName = m_db.connectionName();
        m_db.close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    // Создаем директорию для хранения баз данных приложения
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);

    // Генерируем безопасное имя файла на основе ID пользователя и идентификатора сервера
    QString safeServerId = QString(QCryptographicHash::hash(serverIdentifier.toUtf8(), QCryptographicHash::Md5).toHex());
    QString dbPath = QString("%1/history_u%2_s%3.sqlite")
                         .arg(appDataDir)
                         .arg(currentUserId)
                         .arg(safeServerId);

    m_db = QSqlDatabase::addDatabase("QSQLITE", QString("Conn_User_%1").arg(currentUserId));
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Ошибка открытия SQLite БД:" << m_db.lastError().text();
        return;
    }

    // Создание таблицы сообщений, если её ещё нет
    QSqlQuery query(m_db);
    query.exec("CREATE TABLE IF NOT EXISTS messages ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "sender_id INTEGER NOT NULL, "
               "receiver_id INTEGER NOT NULL, "
               "peer_id INTEGER NOT NULL, "
               "content TEXT NOT NULL, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "is_outgoing BOOLEAN NOT NULL"
               ");");

    query.exec("CREATE INDEX IF NOT EXISTS idx_peer_id ON messages(peer_id);");
    qDebug() << "Путь к БД:" << dbPath;

    // Обновляем список чатов
    updateChatsList();
}

void ChatHandler::saveMessageToDb(int senderId, int receiverId, int peerId, const QString &content, bool isOutgoing) {
    if (!m_db.isOpen()) return;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO messages (sender_id, receiver_id, peer_id, content, is_outgoing) "
                  "VALUES (:sender_id, :receiver_id, :peer_id, :content, :is_outgoing)");
    query.bindValue(":sender_id", senderId);
    query.bindValue(":receiver_id", receiverId);
    query.bindValue(":peer_id", peerId);
    query.bindValue(":content", content);
    query.bindValue(":is_outgoing", isOutgoing);

    if (!query.exec()) {
        qWarning() << "Ошибка сохранения сообщения:" << query.lastError().text();
    } else {
        // Если открыт чат с этим пользователем — сразу обновляем список сообщений
        if (m_activePeerId == peerId) {
            loadChatWithUser(peerId);
        }
        updateChatsList();
    }
}

Q_INVOKABLE void ChatHandler::loadChatWithUser(int peerUserId) {
    m_activePeerId = peerUserId;
    m_currentChatMessages.clear();

    if (!m_db.isOpen()) return;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, sender_id, receiver_id, content, timestamp, is_outgoing "
                  "FROM messages WHERE peer_id = :peer_id ORDER BY id ASC");
    query.bindValue(":peer_id", peerUserId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap msg;
            msg["messageId"] = query.value("id").toInt();
            msg["senderId"] = query.value("sender_id").toInt();
            msg["receiverId"] = query.value("receiver_id").toInt();
            msg["content"] = query.value("content").toString();
            msg["timestamp"] = query.value("timestamp").toString();
            msg["isOutgoing"] = query.value("is_outgoing").toBool();
            m_currentChatMessages.append(msg);
        }
    }

    emit currentChatMessagesChanged();
    emit chatLoadedSuccessfully();
}

void ChatHandler::updateChatsList() {
    m_chatsList.clear();
    if (!m_db.isOpen()) return;

    // Группируем по peer_id для получения списка уникальных собеседников с последним сообщением
    QSqlQuery query(m_db);
    query.exec("SELECT peer_id, content, MAX(timestamp) as last_time "
               "FROM messages GROUP BY peer_id ORDER BY last_time DESC");

    while (query.next()) {
        QVariantMap chat;
        chat["userId"] = query.value("peer_id").toInt();
        chat["lastMessage"] = query.value("content").toString();
        chat["timestamp"] = query.value("last_time").toString();
        // При необходимости имя/аватар можно сопоставлять со списком друзей
        m_chatsList.append(chat);
    }

    emit chatsListChanged();
}

Q_INVOKABLE void ChatHandler::deleteChatHistory(int peerUserId) {
    if (!m_db.isOpen()) return;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM messages WHERE peer_id = :peer_id");
    query.bindValue(":peer_id", peerUserId);

    if (query.exec()) {
        if (m_activePeerId == peerUserId) {
            m_currentChatMessages.clear();
            emit currentChatMessagesChanged();
        }
        updateChatsList();
    }
}

Q_INVOKABLE void ChatHandler::saveAndSendMessage(unsigned int receiverId, const QString &content)
{
    if (content.trimmed().isEmpty()) return;

    // Сохраняем в очередь контекст отправки
    m_pendingMessages.enqueue({receiverId, content});

    // Извещаем ClientController, чтобы тот сформировал и отправил сетевой пакет
    emit sendMessageRequested(receiverId, content);
}

// Приход нового сообщения из сети
void ChatHandler::handleNewMessage(const wire::NewMessageResponse& r) {
    qDebug() << "New message from: " << r.sender_id; 
    saveMessageToDb(r.sender_id, m_currentUserId, r.sender_id, r.content, false);
}

// Подтверждение отправки собственного сообщения
void ChatHandler::handleSendMessage(const wire::SendMessageResponse& r) {
    if (m_pendingMessages.isEmpty()) {
        qWarning() << "Получен SendMessageResponse, но очередь ожиданий пуста!";
        return;
    }

    // Извлекаем первое сообщение из очереди (FIFO)
    PendingMessage pending = m_pendingMessages.dequeue();

    if (r.status == OperationStatus::OK) {
        // Сервер подтвердил успешную доставку -> сохраняем в БД
        saveMessageToDb(m_currentUserId, pending.receiverId, pending.receiverId, pending.content, true);
    } else if(r.status == OperationStatus::UserNotInFriends) {
        emit showErrorToast("Вы не можете написать пользователю который не является вашим другом!");
    } else {
        emit showErrorToast("Произошла ошибка при отправке сообщения!");
    }
}
