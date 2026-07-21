#include "audio_decoder.h"
#include <QDebug>
#include <cstring>

extern "C" {
#include <libavutil/opt.h>
}

AudioDecoder::AudioDecoder(QObject* parent)
  : QObject(parent)
{
}

AudioDecoder::~AudioDecoder()
{
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

bool AudioDecoder::init(int sampleRate, int channels)
{
  mSampleRate = sampleRate;
  mChannels = channels;

  const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_OPUS);
  if (!codec) {
    qWarning() << "Opus decoder not found";
    return false;
  }

  mCodecCtx = avcodec_alloc_context3(codec);
  mCodecCtx->sample_rate = sampleRate;
  mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_MONO;
  if (mChannels != 1) {
    mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
  }
  mCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;

  if (avcodec_open2(mCodecCtx, codec, nullptr) < 0) {
    qWarning() << "Failed to open Opus decoder";
    avcodec_free_context(&mCodecCtx);
    return false;
  }

  mFrame = av_frame_alloc();
  mPacket = av_packet_alloc();
  return true;
}

void AudioDecoder::decode(const QByteArray& encodedFrame)
{
  if (!mCodecCtx) return;

  mPacket->data = (uint8_t*)encodedFrame.data();
  mPacket->size = encodedFrame.size();

  if (avcodec_send_packet(mCodecCtx, mPacket) < 0) {
    av_packet_unref(mPacket);
    return;
  }

  while (avcodec_receive_frame(mCodecCtx, mFrame) == 0) {
    int dataSize = av_samples_get_buffer_size(nullptr, mFrame->ch_layout.nb_channels,
                                              mFrame->nb_samples, AV_SAMPLE_FMT_S16, 1);
    QByteArray pcm(reinterpret_cast<const char*>(mFrame->data[0]), dataSize);
    emit decodedAudioReady(pcm, mSampleRate, mChannels);
    av_frame_unref(mFrame);
  }
  av_packet_unref(mPacket);
}