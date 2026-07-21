#pragma once
#include <QObject>
#include <QCamera>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class VideoCaptureEncoder : public QObject
{
  Q_OBJECT
signals:
  void encodedVideoFrameDone(const QByteArray& encodedFrame, uint32_t durationSamples);

public:
  explicit VideoCaptureEncoder(QObject* parent = nullptr);
  ~VideoCaptureEncoder() override;

  bool start(const QCameraDevice& cameraDevice,
             int width = 1280, int height = 720, int fps = 30);
  void stop();

private slots:
  void onVideoFrameChanged(const QVideoFrame& frame);

private:
  void initEncoder(int width, int height, int fps);
  void encodeAndSend(const QImage& image);

  QCamera*               mCamera = nullptr;
  QMediaCaptureSession*  mSession = nullptr;
  QVideoSink*            mVideoSink = nullptr;

  AVCodecContext*        mCodecCtx = nullptr;
  SwsContext*            mSwsCtx = nullptr;
  AVFrame*               mFrame = nullptr;
  AVPacket*              mPacket = nullptr;

  int     mWidth = 1280;
  int     mHeight = 720;
  int     mFps = 30;
  int64_t mFrameIndex = 0;
};