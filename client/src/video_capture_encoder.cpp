#include "video_capture_encoder.h"
#include <QDebug>
#include <QMediaDevices>

extern "C" {
#include <libavutil/opt.h>
}

VideoCaptureEncoder::VideoCaptureEncoder(QObject* parent)
  : QObject(parent)
{
}

VideoCaptureEncoder::~VideoCaptureEncoder()
{
  stop();
}

bool VideoCaptureEncoder::start(const QCameraDevice& cameraDevice,
                                int width, int height, int fps)
{
  mWidth = width;
  mHeight = height;
  mFps = fps;

  initEncoder(width, height, fps);
  if (!mCodecCtx) {
    qWarning() << "Не удалось инициализировать видеоэнкодер";
    return false;
  }

  mCamera = new QCamera(cameraDevice, this);
  mSession = new QMediaCaptureSession(this);
  mVideoSink = new QVideoSink(this);

  mSession->setCamera(mCamera);
  mSession->setVideoSink(mVideoSink);

  // Подбор формата камеры
  QCameraFormat bestFormat;
  for (const auto& f : cameraDevice.videoFormats()) {
    if (f.resolution().width() == width && f.resolution().height() == height) {
      bestFormat = f;
      break;
    }
  }
  if (bestFormat.isNull() && !cameraDevice.videoFormats().isEmpty()) {
    bestFormat = cameraDevice.videoFormats().first();
  }
  if (!bestFormat.isNull()) {
    mCamera->setCameraFormat(bestFormat);
  }

  connect(mVideoSink, &QVideoSink::videoFrameChanged,
          this, &VideoCaptureEncoder::onVideoFrameChanged);

  mCamera->start();
  return true;
}

void VideoCaptureEncoder::stop()
{
  if (mCamera) {
    mCamera->stop();
    mCamera->deleteLater();
    mCamera = nullptr;
  }
  if (mSession) {
    mSession->deleteLater();
    mSession = nullptr;
  }
  if (mVideoSink) {
    mVideoSink->deleteLater();
    mVideoSink = nullptr;
  }

  if (mSwsCtx) {
    sws_freeContext(mSwsCtx);
    mSwsCtx = nullptr;
  }
  if (mCodecCtx) {
    avcodec_free_context(&mCodecCtx);
  }
  if (mFrame) {
    av_frame_free(&mFrame);
  }
  if (mPacket) {
    av_packet_free(&mPacket);
  }
}

void VideoCaptureEncoder::initEncoder(int width, int height, int fps)
{
  const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!codec) {
    qWarning() << "H.264 энкодер не найден (нужна сборка FFmpeg с libx264)";
    return;
  }

  mCodecCtx = avcodec_alloc_context3(codec);
  if (!mCodecCtx) {
    qWarning() << "Не удалось выделить контекст для видеоэнкодера";
    return;
  }

  mCodecCtx->width = width;
  mCodecCtx->height = height;
  mCodecCtx->time_base = AVRational{1, fps};
  mCodecCtx->framerate = AVRational{fps, 1};
  mCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
  mCodecCtx->bit_rate = 2000000;
  mCodecCtx->gop_size = fps * 2;

  av_opt_set(mCodecCtx->priv_data, "preset", "veryfast", 0);
  av_opt_set(mCodecCtx->priv_data, "tune", "zerolatency", 0);

  if (avcodec_open2(mCodecCtx, codec, nullptr) < 0) {
    qWarning() << "Не удалось открыть H.264 энкодер";
    avcodec_free_context(&mCodecCtx);
    return;
  }

  mFrame = av_frame_alloc();
  if (!mFrame) {
    qWarning() << "Не удалось выделить AVFrame";
    avcodec_free_context(&mCodecCtx);
    return;
  }
  mFrame->format = AV_PIX_FMT_YUV420P;
  mFrame->width  = width;
  mFrame->height = height;
  if (av_frame_get_buffer(mFrame, 32) < 0) {
    qWarning() << "Не удалось выделить буфер для AVFrame";
    av_frame_free(&mFrame);
    avcodec_free_context(&mCodecCtx);
    return;
  }

  mPacket = av_packet_alloc();
  if (!mPacket) {
    qWarning() << "Не удалось выделить AVPacket";
    av_frame_free(&mFrame);
    avcodec_free_context(&mCodecCtx);
  }
}

void VideoCaptureEncoder::onVideoFrameChanged(const QVideoFrame& frame)
{
  if (!frame.isValid() || !mCodecCtx) return;

  QVideoFrame f = frame;
  QImage image = f.toImage();
  if (image.isNull()) {
    return;
  }
  qDebug() << "Video frame captured, size:" << image.size();

  if (image.size() != QSize(mWidth, mHeight)) {
    image = image.scaled(mWidth, mHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  }
  image = image.convertToFormat(QImage::Format_RGB32);

  emit rawVideoFrame(image);
  encodeAndSend(image);
  ++mFrameIndex;
}

void VideoCaptureEncoder::encodeAndSend(const QImage& image)
{
  if (!mSwsCtx) {
    mSwsCtx = sws_getContext(
      mWidth, mHeight, AV_PIX_FMT_RGB32,
      mWidth, mHeight, AV_PIX_FMT_YUV420P,
      SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!mSwsCtx) {
      qWarning() << "Не удалось создать SwsContext";
      return;
    }
  }

  const uint8_t* srcSlices[1] = { image.constBits() };
  int srcStride[1] = { static_cast<int>(image.bytesPerLine()) };

  av_frame_make_writable(mFrame);
  sws_scale(mSwsCtx, srcSlices, srcStride, 0, mHeight,
          mFrame->data, mFrame->linesize);
  mFrame->pts = mFrameIndex;

  if (avcodec_send_frame(mCodecCtx, mFrame) < 0) {
    return;
  }

  // Собираем все пакеты этого кадра в один буфер
  QByteArray combined;
  while (avcodec_receive_packet(mCodecCtx, mPacket) == 0) {
    combined.append(reinterpret_cast<const char*>(mPacket->data), mPacket->size);
    av_packet_unref(mPacket);
  }

  // Если есть данные — отправляем как один кадр
  if (!combined.isEmpty()) {
    uint32_t durationSamples = static_cast<uint32_t>(90000 / mFps);
    encodedVideoFrameDone(combined, durationSamples);
  }
}