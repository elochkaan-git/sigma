#pragma once

#include <QByteArray>
#include <QObject>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

class AudioDecoder : public QObject
{
  Q_OBJECT
public:
  explicit AudioDecoder(QObject* parent = nullptr);
  ~AudioDecoder() override;

  bool init(int sampleRate = 48000, int channels = 1);
  void decode(const QByteArray& encodedFrame);

signals:
  void decodedAudioReady(const QByteArray& pcmData, int sampleRate, int channels);

private:
  AVCodecContext* mCodecCtx = nullptr;
  AVFrame*        mFrame = nullptr;
  AVPacket*       mPacket = nullptr;
  int             mSampleRate = 48000;
  int             mChannels = 1;
};