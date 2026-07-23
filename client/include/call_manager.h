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
  // QML_ELEMENT
  // QML_UNCREATABLE("CallManager is read-only")
  Q_PROPERTY(CallState callState READ callState NOTIFY callStateChanged)
  Q_PROPERTY(bool isVideoEnabled READ isVideoEnabled NOTIFY videoEnabledChanged)
  Q_PROPERTY(unsigned int callerId READ callerId NOTIFY callerIdChanged)

public:
  enum class CallState {
    Idle,         // Нет активного вызова
    Incoming,     // Входящий звонок (Принять / Отклонить)
    Outgoing,     // Исходящий звонок (Ожидание ответа / Отмена)
    Connected     // Разговор начат (Аудио / Видео окно)
  };
  Q_ENUM(CallState)

  explicit CallManager(Transport* transport, QObject* parent = nullptr);
  ~CallManager() override;

  CallState callState() const { return mCallState; }
  bool isVideoEnabled() const { return mVideoEnabled; }
  unsigned int callerId() const { return mCallerId; }

  /**
   * @brief Установка устройств пользователя. Если пусто,
   * то будут использованы первые попавшиеся
   * 
   * @param devices 
   */
  void setDevices(const MediaDevices& devices);
  /**
   * @brief Инициализация менеджера. Обязательно вызвать
   * перед выполнением каких-либо действий!
   * 
   * @return true 
   * @return false 
   */
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
  void handleRejectCallResponse(const wire::RejectCallResponse& r);
  void handleCallAccepted(const wire::CallAcceptedResponse& r);
  void handleCallRejected(const wire::CallRejectedResponse& r);
  void handleCallEnded(const wire::CallEndedResponse& r);
  void handleSdp(const wire::SdpResponse& r);
  void handleIceCandidate(const wire::IceCandidateResponse& r);
  void handleGetTurnCredentials(const QString& ip, const wire::GetTurnCredentialsResponse& r);

signals:
  void callStateChanged(CallState newState);
  void videoEnabledChanged(bool enabled);
  void callerIdChanged(unsigned int callerId);
  /**
   * @brief Сигнал при установке соединения между клиентами
   */
  void callEstablished();
  /**
   * @brief Сигнал при завершении звонка
   */
  void callClosed();
  /**
   * @brief Сигнал при локальной ошибке WebRTC
   * 
   * @param reason причина ошибки
   */
  void callFailed(const QString& reason);

  /**
   * @brief Сигнал с декодированными аудио данными
   * 
   * @param pcmData непосредственно аудио
   * @param sampleRate частота дискретизации
   * @param channels количество каналов (1 - моно, 2 - стерео)
   */
  void decodedAudioReady(const QByteArray& pcmData, int sampleRate, int channels);
  /**
   * @brief Сигнал с декодированным кадром видео
   * 
   * @param image кадр видео
   */
  void remoteVideoFrameReady(const QImage& image);
  void localVideoFrameReady(const QImage &frame);

  void showErrorToast(const QString &message);

private slots:
  void onCallEstablished();
  void onCallClosed();
  void onCallFailed(const QString& reason);

  // Подключение входящих закодированных данных к декодерам
  void onAudioFrameReceived(const QByteArray& encodedFrame);
  void onVideoFrameReceived(const QByteArray& encodedFrame);

private:
  void setCallState(CallState state);
  void startCapturing();
  void stopCapturing();
  void initDecoders(int audioSampleRate = 48000, int videoWidth = 640, int videoHeight = 480);
  void setCallerId(unsigned int id);

  Transport* mTransport = nullptr;
  MediaDevices mDevices;
  bool mInitialized = false;
  unsigned int mCallerId = 0;

  CallState mCallState = CallState::Idle;

  std::unique_ptr<WebRtcWrapper> mWebRtc;
  std::unique_ptr<AudioCaptureEncoder> mAudioCapture;
  std::unique_ptr<VideoCaptureEncoder> mVideoCapture;

  // Декодеры для входящего потока
  std::unique_ptr<AudioDecoder> mAudioDecoder;
  std::unique_ptr<VideoDecoder> mVideoDecoder;

  bool mVideoEnabled = false;
};
