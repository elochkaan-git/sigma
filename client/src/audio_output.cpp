#include "audio_output.h"
#include <QDebug>
#include <QMediaDevices>

AudioOutput::AudioOutput(QObject *parent) : QObject(parent) {}

AudioOutput::~AudioOutput()
{
    stop();
}

bool AudioOutput::init(int sampleRate, int channels)
{
    if (m_audioSink) {
        stop();
    }

    // 1. Получаем устройство вывода по умолчанию
    QAudioDevice device = QMediaDevices::defaultAudioOutput();

    // 2. Настраиваем желаемый формат
    m_format.setSampleRate(sampleRate);
    m_format.setChannelCount(channels);
    m_format.setSampleFormat(QAudioFormat::Int16);

    // 3. Проверяем поддержку формата устройством
    if (!device.isFormatSupported(m_format)) {
        qWarning() << "Audio format not supported, using nearest format";
        m_format = device.preferredFormat(); // используем предпочтительный формат устройства[reference:2][reference:3]
        // (Опционально) обновляем параметры для логов
        sampleRate = m_format.sampleRate();
        channels = m_format.channelCount();
    }

    // 4. Создаем QAudioSink с устройством и форматом
    m_audioSink = new QAudioSink(device, m_format, this);

    // 5. Запускаем и сохраняем QIODevice для записи
    m_audioOutput = m_audioSink->start();
    if (!m_audioOutput) {
        qWarning() << "Failed to start audio sink";
        return false;
    }

    // 6. Подключаем сигнал изменения состояния
    connect(m_audioSink, &QAudioSink::stateChanged, this, &AudioOutput::stateChanged);

    return true;
}

void AudioOutput::writePCM(const QByteArray& pcmData)
{
    if (!m_audioOutput || !m_audioSink) {
        qWarning() << "Audio output not initialized";
        return;
    }

    // Если остановился – перезапускаем
    if (m_audioSink->state() == QAudio::StoppedState) {
        qDebug() << "Audio sink stopped, restarting";
        m_audioOutput = m_audioSink->start();
        if (!m_audioOutput) {
            qWarning() << "Failed to restart audio sink";
            return;
        }
    }

    // Проверка размера (должен быть кратен channels * sizeof(int16_t))
    const int bytesPerSample = 2; // Int16
    int expectedSize = m_format.channelCount() * bytesPerSample;
    if (pcmData.size() % expectedSize != 0) {
        qWarning() << "PCM data size not aligned:" << pcmData.size() << "bytes, expected multiple of" << expectedSize;
        // Можно попытаться обрезать до целого числа фреймов
    }

    qint64 written = m_audioOutput->write(pcmData);
    if (written != pcmData.size()) {
        qWarning() << "Written only" << written << "of" << pcmData.size() << "bytes";
    } else {
        qDebug() << "Audio written:" << written << "bytes";
    }
}

void AudioOutput::stop()
{
    if (m_audioSink) {
        m_audioSink->stop();
        m_audioSink->reset();
        delete m_audioSink;
        m_audioSink = nullptr;
        m_audioOutput = nullptr;
    }
}

void AudioOutput::reset()
{
    if (m_audioSink) {
        m_audioSink->reset();
    }
}