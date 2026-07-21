#include "audio_decoder.h"
#include <QDebug>
#include <cstring>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
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
  if (mSwrCtx) {
    swr_free(&mSwrCtx);
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
  if (!mCodecCtx) {
    qWarning() << "Failed to allocate codec context";
    return false;
  }

  mCodecCtx->sample_rate = sampleRate;
  if (channels == 1) {
    mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_MONO;
  } else {
    mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
  }

  mCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
  if (avcodec_open2(mCodecCtx, codec, nullptr) < 0) {
    qWarning() << "Failed to open Opus decoder";
    avcodec_free_context(&mCodecCtx);
    return false;
  }

  AVSampleFormat actualFmt = mCodecCtx->sample_fmt;
  int actualChannels = mCodecCtx->ch_layout.nb_channels;
  qDebug() << "Decoder actual format:" << av_get_sample_fmt_name(actualFmt)
           << "channels:" << actualChannels;

  if (actualFmt != AV_SAMPLE_FMT_S16 || actualChannels != channels) {
    qDebug() << "Creating swr context to convert to S16, channels:" << channels;

    AVChannelLayout inLayout = mCodecCtx->ch_layout;

    AVChannelLayout outLayout;
    if (channels == 1) {
      mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_MONO;
    } else {
      mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    }

    mSwrCtx = swr_alloc();
    if (!mSwrCtx) {
      qWarning() << "swr_alloc failed";
      return false;
    }

    av_opt_set_chlayout(mSwrCtx, "in_chlayout", &inLayout, 0);
    av_opt_set_chlayout(mSwrCtx, "out_chlayout", &outLayout, 0);
    av_opt_set_int(mSwrCtx, "in_sample_rate", mCodecCtx->sample_rate, 0);
    av_opt_set_int(mSwrCtx, "out_sample_rate", sampleRate, 0);
    av_opt_set_sample_fmt(mSwrCtx, "in_sample_fmt", actualFmt, 0);
    av_opt_set_sample_fmt(mSwrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    if (swr_init(mSwrCtx) < 0) {
      qWarning() << "swr_init failed";
      swr_free(&mSwrCtx);
      mSwrCtx = nullptr;
      return false;
    }
    qDebug() << "swr initialized successfully";
  }

  mFrame = av_frame_alloc();
  mPacket = av_packet_alloc();
  return true;
}

void AudioDecoder::decode(const QByteArray& encodedFrame)
{
  if (!mCodecCtx) {
    qWarning() << "Decoder not initialized, ignoring frame";
    return;
  }

  mPacket->data = (uint8_t*)encodedFrame.data();
  mPacket->size = encodedFrame.size();

  if (avcodec_send_packet(mCodecCtx, mPacket) < 0) {
    av_packet_unref(mPacket);
    return;
  }

  while (avcodec_receive_frame(mCodecCtx, mFrame) == 0) {
    QByteArray pcm;

    if (mSwrCtx) {
      // Конвертируем через swr
      int outSamples = av_rescale_rnd(mFrame->nb_samples,
                                      mSampleRate,
                                      mCodecCtx->sample_rate,
                                      AV_ROUND_UP);
      uint8_t* outBuffer[1];
      int outBufferSize = av_samples_get_buffer_size(nullptr, mChannels,
                                                      outSamples, AV_SAMPLE_FMT_S16, 1);
      outBuffer[0] = (uint8_t*)av_malloc(outBufferSize);
      if (!outBuffer[0]) {
        av_frame_unref(mFrame);
        continue;
      }

      int samplesConverted = swr_convert(mSwrCtx, outBuffer, outSamples,
                                          (const uint8_t**)mFrame->data, mFrame->nb_samples);
      if (samplesConverted > 0) {
        int dataSize = samplesConverted * mChannels * sizeof(int16_t);
        pcm = QByteArray(reinterpret_cast<const char*>(outBuffer[0]), dataSize);
      }
      av_free(outBuffer[0]);
    } else {
      // Без конвертации (предполагаем S16 упакованный)
      int dataSize = av_samples_get_buffer_size(nullptr,
                                                mFrame->ch_layout.nb_channels,
                                                mFrame->nb_samples,
                                                AV_SAMPLE_FMT_S16, 1);
      pcm = QByteArray(reinterpret_cast<const char*>(mFrame->data[0]), dataSize);
    }

    if (!pcm.isEmpty()) {
      emit decodedAudioReady(pcm, mSampleRate, mChannels);
    }
    av_frame_unref(mFrame);
  }
  av_packet_unref(mPacket);
}