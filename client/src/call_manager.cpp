#include "call_manager.h"
#include "audio_capture_encoder.h"
#include "video_capture_encoder.h"
#include "webrtc.h"
#include "audio_decoder.h"
#include "video_decoder.h"
#include <QDebug>
#include <QMediaDevices>

CallManager::CallManager(Transport* transport, QObject* parent)
  : QObject(parent)
  , mTransport(transport)
{
  // Создаём компоненты
  mWebRtc.reset(new WebRtcWrapper(mTransport, this));
  mAudioCapture.reset(new AudioCaptureEncoder(this));
  mVideoCapture.reset(new VideoCaptureEncoder(this));

  // Соединяем сигналы захвата с отправкой через WebRTC
  connect(mAudioCapture.get(), &AudioCaptureEncoder::encodedAudioFrameDone,
          mWebRtc.get(), &WebRtcWrapper::sendAudioFrame);
  connect(mVideoCapture.get(), &VideoCaptureEncoder::encodedVideoFrameDone,
          mWebRtc.get(), &WebRtcWrapper::sendVideoFrame);

  // Пробрасываем сигналы состояния звонка
  connect(mWebRtc.get(), &WebRtcWrapper::callEstablished,
          this, &CallManager::onCallEstablished);
  connect(mWebRtc.get(), &WebRtcWrapper::callClosed,
          this, &CallManager::onCallClosed);
  connect(mWebRtc.get(), &WebRtcWrapper::callFailed,
          this, &CallManager::onCallFailed);

  // Подключаем входящие закодированные данные к слотам-обработчикам
  connect(mWebRtc.get(), &WebRtcWrapper::audioFrameReceived,
          this, &CallManager::onAudioFrameReceived);
  connect(mWebRtc.get(), &WebRtcWrapper::videoFrameReceived,
          this, &CallManager::onVideoFrameReceived);

  // Создаём декодеры (пока без инициализации, инициализируем при установке звонка)
  mAudioDecoder.reset(new AudioDecoder(this));
  mVideoDecoder.reset(new VideoDecoder(this));

  // Пробрасываем сигналы от декодеров наружу
  connect(mAudioDecoder.get(), &AudioDecoder::decodedAudioReady,
          this, &CallManager::decodedAudioReady);
  connect(mVideoDecoder.get(), &VideoDecoder::decodedVideoReady,
          this, &CallManager::decodedVideoReady);
}

CallManager::~CallManager()
{
  stopCapturing();
}

void CallManager::setDevices(const MediaDevices& devices)
{
  mDevices = devices;
}

bool CallManager::initialize()
{
  if (mInitialized) {
    return true;
  }

  if (mDevices.audio.isNull()) {
    auto audioDevices = QMediaDevices::audioInputs();
    if (!audioDevices.isEmpty()) mDevices.audio = audioDevices.first();
  }
  if (mDevices.video.isNull()) {
    auto videoDevices = QMediaDevices::videoInputs();
    if (!videoDevices.isEmpty()) mDevices.video = videoDevices.first();
  }

  if (mDevices.audio.isNull() || mDevices.video.isNull()) {
    qWarning() << "Не удалось найти аудио- или видеоустройства";
    return false;
  }

  mInitialized = true;
  return true;
}

void CallManager::startCall(unsigned int calleeId, bool withVideo)
{
  if (!mInitialized && !initialize()) {
    qWarning() << "CallManager не инициализирован";
    return;
  }
  mVideoEnabled = withVideo;
  mWebRtc->startCall(calleeId, withVideo);
}

void CallManager::acceptCall()
{
  mWebRtc->acceptCall();
}

void CallManager::rejectCall()
{
  mWebRtc->rejectCall();
}

void CallManager::endCall()
{
  mWebRtc->endCall();
}

// --- Обработчики ответов от сервера ---
void CallManager::handleStartCallResponse(const wire::StartCallResponse& r) {
  mWebRtc->handleStartCallResponse(r);
}

void CallManager::handleIncomingCall(const wire::IncomingCallResponse& r)   {
  mWebRtc->handleIncomingCall(r);
}

void CallManager::handleAcceptCallResponse(const wire::AcceptCallResponse& r) {
  mWebRtc->handleAcceptCallResponse(r);
}

void CallManager::handleCallAccepted(const wire::CallAcceptedResponse& r)   {
  mWebRtc->handleCallAccepted(r);
}

void CallManager::handleCallRejected(const wire::CallRejectedResponse& r)   {
  mWebRtc->handleCallRejected(r);
}

void CallManager::handleCallEnded(const wire::CallEndedResponse& r)         {
  mWebRtc->handleCallEnded(r);
}

void CallManager::handleSdp(const wire::SdpResponse& r)                     {
  mWebRtc->handleRemoteSdp(r);
}

void CallManager::handleIceCandidate(const wire::IceCandidateResponse& r)   {
  mWebRtc->handleRemoteIceCandidate(r);
}

// --- Слоты для состояния звонка ---
void CallManager::onCallEstablished()
{
  qDebug() << "Call established, initializing decoders and starting capture";
  initDecoders(48000, 640, 480);
  startCapturing();
  emit callEstablished();
}

void CallManager::onCallClosed()
{
  qDebug() << "Call closed, stopping capture";
  stopCapturing();
  emit callClosed();
}

void CallManager::onCallFailed(const QString& reason)
{
  qDebug() << "Call failed:" << reason;
  stopCapturing();
  emit callFailed(reason);
}

// --- Обработка входящих закодированных данных ---
void CallManager::onAudioFrameReceived(const QByteArray& encodedFrame)
{
  mAudioDecoder->decode(encodedFrame);
}

void CallManager::onVideoFrameReceived(const QByteArray& encodedFrame)
{
  mVideoDecoder->decode(encodedFrame);
}

// --- Приватные методы ---
void CallManager::initDecoders(int audioSampleRate, int videoWidth, int videoHeight)
{
  if (!mAudioDecoder->init(audioSampleRate, 1)) {
    qWarning() << "Не удалось инициализировать аудиодекодер";
  }
  if (!mVideoDecoder->init(videoWidth, videoHeight)) {
    qWarning() << "Не удалось инициализировать видеодекодер";
  }
}

void CallManager::startCapturing()
{
  if (!mAudioCapture->start(mDevices.audio, 48000, 1)) {
    qWarning() << "Не удалось запустить аудиозахват";
  }

  if (mVideoEnabled) {
    if (!mVideoCapture->start(mDevices.video, 640, 480, 30)) {
      qWarning() << "Не удалось запустить видеозахват";
    }
  }
}

void CallManager::stopCapturing()
{
  mAudioCapture->stop();
  mVideoCapture->stop();
}