#pragma once
#include <QQuickImageProvider>
#include <QImage>
#include <QByteArray>
#include <QMap>

class AvatarImageProvider : public QQuickImageProvider
{
public:
    AvatarImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    // Метод, который вызывается движком QML, когда Image хочет нарисовать "image://avatars/<id>"
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    // Сохраняем чистый Base64 от сервера как QImage в C++ память
    void updateAvatar(const QString &userId, const QString &base64Data);

private:
    QMap<QString, QImage> m_avatars;
};
