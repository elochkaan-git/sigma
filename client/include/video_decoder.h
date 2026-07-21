#pragma once
#include <QObject>
#include <QImage>
#include <QByteArray>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class VideoDecoder : public QObject
{
  Q_OBJECT
public:
  explicit VideoDecoder(QObject* parent = nullptr);
  ~VideoDecoder() override;

  bool init(int width = 1280, int height = 720);
  void decode(const QByteArray& encodedFrame);

signals:
  void decodedVideoReady(const QImage& image);

private:
  AVCodecContext* mCodecCtx = nullptr;
  SwsContext*     mSwsCtx = nullptr;
  AVFrame*        mFrame = nullptr;
  AVFrame*        mRgbFrame = nullptr;
  AVPacket*       mPacket = nullptr;

  int mWidth = 1280;
  int mHeight = 720;
  uint8_t* mRgbBuffer = nullptr;
};