#pragma once
#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QObject>

class VideoImageProvider : public QQuickImageProvider
{
    Q_OBJECT
public:
    // Явный вызов конструктора базового класса QQuickImageProvider
    VideoImageProvider(QObject*) : QQuickImageProvider(QQuickImageProvider::Image)
    {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        qDebug() << "VideoImageProvider::requestImage called with id:" << id;
        QMutexLocker locker(&m_mutex);
        if (size) *size = m_image.size();
        if (!m_image.isNull() && requestedSize.isValid()) {
            return m_image.scaled(requestedSize, Qt::KeepAspectRatio);
        }
        return m_image;
    }

    void updateFrame(const QImage &frame) {
        QMutexLocker locker(&m_mutex);
        m_image = frame;
        emit imageChanged();   // теперь сигнал существует
    }

signals:
    void imageChanged();

private:
    QImage m_image;
    QMutex m_mutex;
};