#pragma once
#include <QObject>
#include <QAudioSource>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QIODevice>
#include <QByteArray>

extern "C" {
#include <libavcodec/avcodec.h>
}

class AudioCaptureEncoder : public QObject
{
  Q_OBJECT
signals:
    void encodedAudioFrameDone(const QByteArray& encodedFrame, uint32_t durationSamples);

public:
    explicit AudioCaptureEncoder(QObject* parent = nullptr);
    ~AudioCaptureEncoder() override;

    bool start(const QAudioDevice& inputDevice, int sampleRate = 48000, int channels = 1);
    void stop();

private slots:
    void onReadyRead();

private:
    void initEncoder();
    void encodeAndSend(const int16_t* pcm, int samples);

    QAudioSource* mAudioSource = nullptr;
    QIODevice*    mIODevice = nullptr; // pull-режим чтения PCM

    AVCodecContext* mCodecCtx = nullptr;
    AVFrame*        mFrame = nullptr;
    AVPacket*       mPacket = nullptr;

    QByteArray mPcmBuffer;
    int        mSampleRate = 48000;
    int        mChannels   = 1;
    int        mFrameSamples = 960;
    int64_t    mSamplesEncoded = 0;
};