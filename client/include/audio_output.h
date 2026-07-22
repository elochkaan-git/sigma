#pragma once
#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>

class AudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject *parent = nullptr);
    ~AudioOutput();

    bool init(int sampleRate = 48000, int channels = 1);
    void writePCM(const QByteArray& pcmData);
    void stop();
    void reset();

signals:
    void stateChanged(QAudio::State state);

private:
    QAudioSink* m_audioSink = nullptr;
    QIODevice* m_audioOutput = nullptr;
    QAudioFormat m_format;
};