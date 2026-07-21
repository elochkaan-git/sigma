#include "video_decoder.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

VideoDecoder::VideoDecoder(QObject* parent)
  : QObject(parent)
{
}

VideoDecoder::~VideoDecoder()
{
  if (mCodecCtx) avcodec_free_context(&mCodecCtx);
  if (mFrame) av_frame_free(&mFrame);
  if (mRgbFrame) av_frame_free(&mRgbFrame);
  if (mPacket) av_packet_free(&mPacket);
  if (mSwsCtx) sws_freeContext(mSwsCtx);
  av_free(mRgbBuffer);
}

bool VideoDecoder::init(int width, int height)
{
  mWidth = width;
  mHeight = height;

  const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!codec) {
    qWarning() << "H.264 decoder not found";
    return false;
  }

  mCodecCtx = avcodec_alloc_context3(codec);
  mCodecCtx->width = width;
  mCodecCtx->height = height;
  mCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

  if (avcodec_open2(mCodecCtx, codec, nullptr) < 0) {
    qWarning() << "Failed to open H.264 decoder";
    avcodec_free_context(&mCodecCtx);
    return false;
  }

  mFrame = av_frame_alloc();
  mRgbFrame = av_frame_alloc();
  mPacket = av_packet_alloc();

  int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 1);
  mRgbBuffer = (uint8_t*)av_malloc(numBytes);
  av_image_fill_arrays(mRgbFrame->data, mRgbFrame->linesize, mRgbBuffer,
                        AV_PIX_FMT_RGB32, width, height, 1);

  return true;
}

void VideoDecoder::decode(const QByteArray& encodedFrame)
{
  if (!mCodecCtx) {
    return;
  }

  mPacket->data = (uint8_t*)encodedFrame.data();
  mPacket->size = encodedFrame.size();

  if (avcodec_send_packet(mCodecCtx, mPacket) < 0) {
    av_packet_unref(mPacket);
    return;
  }

  while (avcodec_receive_frame(mCodecCtx, mFrame) == 0) {
    // Конвертируем YUV -> RGB32
    if (!mSwsCtx) {
      mSwsCtx = sws_getContext(mWidth, mHeight, mCodecCtx->pix_fmt,
                                mWidth, mHeight, AV_PIX_FMT_RGB32,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);
    }
    sws_scale(mSwsCtx, mFrame->data, mFrame->linesize, 0, mHeight,
              mRgbFrame->data, mRgbFrame->linesize);

    QImage image(mRgbBuffer, mWidth, mHeight, QImage::Format_RGB32);
    emit decodedVideoReady(image.copy());
    av_frame_unref(mFrame);
  }
  av_packet_unref(mPacket);
}