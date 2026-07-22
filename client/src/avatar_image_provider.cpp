#include "avatar_image_provider.h"

QImage AvatarImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    if (m_avatars.contains(id) && !m_avatars.value(id).isNull()) {
        QImage img = m_avatars.value(id);
        
        if (size) {
            *size = img.size();
        }

        // Если QML запросил конкретный размер, масштабируем
        if (requestedSize.width() > 0 && requestedSize.height() > 0) {
            return img.scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        
        return img;
    }

    // Фоллбэк если ID не найден
    QImage defaultAvatar(":/Main/assets/person.png");
    if (size) *size = defaultAvatar.size();
    return defaultAvatar;
}

void AvatarImageProvider::updateAvatar(const QString &userId, const QString &base64Data)
{
    if (base64Data.isEmpty()) {
        m_avatars.remove(userId);
        return;
    }

    QString cleanBase64 = base64Data;

    int headerIndex = cleanBase64.indexOf("base64,");
    if (headerIndex != -1) {
        cleanBase64 = cleanBase64.mid(headerIndex + 7);
    }

    QByteArray ba = QByteArray::fromBase64(base64Data.toLatin1());
    QImage img;
    if (img.loadFromData(ba)) {
        m_avatars[userId] = img;
        qDebug() << "[AvatarProvider] Успешно загружена аватарка для ID:" << userId << "размер:" << img.size();
    } else {
        qWarning() << "[AvatarProvider] Ошибка декодирования QImage из Base64 для ID:" << userId;
    }
}
