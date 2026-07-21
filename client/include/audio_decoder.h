#pragma once
#include <QObject>
#include <QByteArray>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

class AudioDecoder : public QObject
{
  Q_OBJECT
public:
  explicit AudioDecoder(QObject* parent = nullptr);
  ~AudioDecoder() override;

  bool init(int sampleRate, int channels);
  void decode(const QByteArray& encodedFrame);

signals:
  void decodedAudioReady(const QByteArray& pcmData, int sampleRate, int channels);

private:
  AVCodecContext* mCodecCtx = nullptr;
  SwrContext*     mSwrCtx = nullptr;
  AVFrame*        mFrame = nullptr;
  AVPacket*       mPacket = nullptr;
  int             mSampleRate = 48000;
  int             mChannels = 1;
};