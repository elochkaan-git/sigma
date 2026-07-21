#pragma once
#include <QObject>
#include <memory>
#include <QAudioDevice>
#include <QCameraDevice>
#include <QImage>

#include "responses.h"

class Transport;
class AudioCaptureEncoder;
class VideoCaptureEncoder;
class WebRtcWrapper;
class AudioDecoder;
class VideoDecoder;

struct MediaDevices {
  QAudioDevice audio;
  QCameraDevice video;
};

class CallManager : public QObject
{
  Q_OBJECT
public:
  explicit CallManager(Transport* transport, QObject* parent = nullptr);
  ~CallManager() override;

  void setDevices(const MediaDevices& devices);
  bool initialize();

  // Действия пользователя
  void startCall(unsigned int calleeId, bool withVideo = false);
  void acceptCall();
  void rejectCall();
  void endCall();

  // Обработчики ответов от сервера (делегируются в WebRtcWrapper)
  void handleStartCallResponse(const wire::StartCallResponse& r);
  void handleIncomingCall(const wire::IncomingCallResponse& r);
  void handleAcceptCallResponse(const wire::AcceptCallResponse& r);
  void handleCallAccepted(const wire::CallAcceptedResponse& r);
  void handleCallRejected(const wire::CallRejectedResponse& r);
  void handleCallEnded(const wire::CallEndedResponse& r);
  void handleSdp(const wire::SdpResponse& r);
  void handleIceCandidate(const wire::IceCandidateResponse& r);

signals:
  // Сигналы состояния звонка
  void callEstablished();
  void callClosed();
  void callFailed(const QString& reason);

  // Декодированные медиа-данные для клиента
  void decodedAudioReady(const QByteArray& pcmData, int sampleRate, int channels);
  void decodedVideoReady(const QImage& image);

private slots:
  void onCallEstablished();
  void onCallClosed();
  void onCallFailed(const QString& reason);

  // Подключение входящих закодированных данных к декодерам
  void onAudioFrameReceived(const QByteArray& encodedFrame);
  void onVideoFrameReceived(const QByteArray& encodedFrame);

private:
  void startCapturing();
  void stopCapturing();
  void initDecoders(int audioSampleRate = 48000, int videoWidth = 640, int videoHeight = 480);

  Transport* mTransport = nullptr;
  MediaDevices mDevices;
  bool mInitialized = false;

  std::unique_ptr<WebRtcWrapper> mWebRtc;
  std::unique_ptr<AudioCaptureEncoder> mAudioCapture;
  std::unique_ptr<VideoCaptureEncoder> mVideoCapture;

  // Декодеры для входящего потока
  std::unique_ptr<AudioDecoder> mAudioDecoder;
  std::unique_ptr<VideoDecoder> mVideoDecoder;

  bool mVideoEnabled = false;
};