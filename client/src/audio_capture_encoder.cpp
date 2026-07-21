#include "audio_capture_encoder.h"

#include <QDebug>
#include <QMediaDevices>
#include <cstring>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

AudioCaptureEncoder::AudioCaptureEncoder(QObject* parent)
  : QObject(parent)
{
}

AudioCaptureEncoder::~AudioCaptureEncoder()
{
  stop();
}

bool AudioCaptureEncoder::start(const QAudioDevice& inputDevice, int sampleRate, int channels)
{
  mSampleRate = sampleRate;
  mChannels = channels;

  QAudioFormat format;
  format.setSampleRate(mSampleRate);
  format.setChannelCount(mChannels);
  format.setSampleFormat(QAudioFormat::Int16);

  if (!inputDevice.isFormatSupported(format)) {
    qWarning() << "Запрошенный формат аудио не поддерживается устройством, "
                    "используем preferredFormat()";
    format = inputDevice.preferredFormat();
    mSampleRate = format.sampleRate();
    mChannels = format.channelCount();
  }

  initEncoder();
  if (!mCodecCtx) {
    return false;
  }

  mAudioSource = new QAudioSource(inputDevice, format, this);
  mIODevice = mAudioSource->start(); // pull-режим: получаем QIODevice для чтения PCM
  if (!mIODevice) {
    qWarning() << "Не удалось запустить захват аудио";
    return false;
  }

  connect(mIODevice, &QIODevice::readyRead, this, &AudioCaptureEncoder::onReadyRead);
  return true;
}

void AudioCaptureEncoder::stop()
{
  if (mAudioSource) {
    mAudioSource->stop();
    mAudioSource->deleteLater();
    mAudioSource = nullptr;
    mIODevice = nullptr;
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

void AudioCaptureEncoder::initEncoder()
{
  const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);
  if (!codec) {
    qWarning() << "Кодек Opus не найден в сборке FFmpeg";
    return;
  }

  mCodecCtx = avcodec_alloc_context3(codec);
  mCodecCtx->sample_rate = mSampleRate;
  mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_MONO;
  if (mChannels != 1) {
    mCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
  }
  mCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
  mCodecCtx->bit_rate = 32000;
  // 2049 = OPUS_APPLICATION_VOIP
  av_opt_set_int(mCodecCtx->priv_data, "application", 2049, 0);

  if (avcodec_open2(mCodecCtx, codec, nullptr) < 0) {
    qWarning() << "Не удалось открыть Opus энкодер";
    avcodec_free_context(&mCodecCtx);
    return;
  }

  mFrameSamples = (mCodecCtx->frame_size > 0) ? mCodecCtx->frame_size : 960;

  mFrame = av_frame_alloc();
  mFrame->format = mCodecCtx->sample_fmt;
  mFrame->ch_layout = mCodecCtx->ch_layout;
  mFrame->sample_rate = mSampleRate;
  mFrame->nb_samples = mFrameSamples;
  av_frame_get_buffer(mFrame, 0);

  mPacket = av_packet_alloc();
}

void AudioCaptureEncoder::onReadyRead()
{
  mPcmBuffer.append(mIODevice->readAll());

  const int bytesPerFrame = mFrameSamples * mChannels * static_cast<int>(sizeof(int16_t));
  while (mPcmBuffer.size() >= bytesPerFrame) {
    const int16_t* pcm = reinterpret_cast<const int16_t*>(mPcmBuffer.constData());
    encodeAndSend(pcm, mFrameSamples);
    mPcmBuffer.remove(0, bytesPerFrame);
  }
}

void AudioCaptureEncoder::encodeAndSend(const int16_t* pcm, int samples)
{
  av_frame_make_writable(mFrame);
  std::memcpy(mFrame->data[0], pcm, static_cast<size_t>(samples) * mChannels * sizeof(int16_t));
  mFrame->pts = mSamplesEncoded;

  if (avcodec_send_frame(mCodecCtx, mFrame) < 0) {
    return;
  }

  while (avcodec_receive_packet(mCodecCtx, mPacket) == 0) {
    QByteArray encoded(reinterpret_cast<const char*>(mPacket->data), mPacket->size);
    encodedAudioFrameDone(encoded, static_cast<uint32_t>(samples));
    av_packet_unref(mPacket);
  }

  mSamplesEncoded += samples;
}