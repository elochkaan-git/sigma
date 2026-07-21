#pragma once

#include "responses.h"
#include "transport.h"

#include <QByteArray>
#include <QObject>
#include <QUuid>
#include <rtc/configuration.hpp>

#include <memory>
#include <vector>

namespace rtc {
  class PeerConnection;
  class Track;
  class RtpPacketizationConfig;
}

class WebRtcWrapper : public QObject
{
  Q_OBJECT

public:
  explicit WebRtcWrapper(Transport* transport, QObject* parent = nullptr);
  ~WebRtcWrapper() override;

  /**
   * @brief Задать список ICE-серверов (STUN/TURN) для последующих звонков.
   * Должен вызываться до startCall()/acceptCall() (например, сразу после
   * получения кредов TURN от сервера). Публичный STUN-сервер Google
   * оставлен как запасной вариант только для разработки, для прода нужен
   * свой TURN.
   */
  void setIceServers(const std::vector<rtc::IceServer>& servers);

  void startCall(unsigned int calleeId, bool withVideo = false);
  void acceptCall();
  void rejectCall();
  void endCall();

  void handleStartCallResponse(const wire::StartCallResponse& response);
  void handleIncomingCall(const wire::IncomingCallResponse& response);
  void handleCallAccepted(const wire::CallAcceptedResponse& response);
  void handleCallRejected(const wire::CallRejectedResponse& response);
  void handleAcceptCallResponse(const wire::AcceptCallResponse& response);

  void handleRemoteSdp(const wire::SdpResponse& response);
  void handleRemoteIceCandidate(const wire::IceCandidateResponse& response);

  void handleCallEnded(const wire::CallEndedResponse& response);

  /**
   * @brief Отправить уже закодированный аудио-кадр (например, Opus) второй
   * стороне звонка. Кодирование сюда не входит — ожидается на входе готовый
   * битстрим кодека.
   * @param durationSamples длительность кадра в сэмплах (при 48kHz/20ms это
   * 960), используется для инкремента RTP-таймстампа
   */
  void sendAudioFrame(const QByteArray& encodedFrame, uint32_t durationSamples);
  /**
   * @brief Отправить уже закодированный видео-кадр (H.264 access unit)
   * второй стороне звонка. Кодирование сюда не входит.
   * @param durationSamples длительность кадра в тактах клока 90kHz (для
   * 30fps это 3000)
   */
  void sendVideoFrame(const QByteArray& encodedFrame, uint32_t durationSamples);

signals:
  void requestSendCommand(const Command& command);

  /**
   * @brief PeerConnection перешёл в состояние Connected — можно показывать
   * экран активного звонка
   */
  void callEstablished();
  /**
   * @brief Звонок прекращён (локально или удалённо, успешно или из-за
   * ошибки соединения). Испускается ровно один раз на звонок, после этого
   * состояние обёртки снова Idle.
   */
  void callClosed();
  /**
   * @brief Локальная ошибка WebRTC-уровня (например, PeerConnection::State
   * стал Failed) — сигнализируется отдельно от callClosed, чтобы Client мог
   * показать причину пользователю
   */
  void callFailed(const QString& reason);

  /**
   * @brief Получен и реассемблирован закодированный аудио-кадр от второй
   * стороны — передать декодеру
   */
  void audioFrameReceived(const QByteArray& encodedFrame);
  /**
   * @brief Получен и реассемблирован закодированный видео-кадр от второй
   * стороны — передать декодеру
   */
  void videoFrameReceived(const QByteArray& encodedFrame);

private:
  enum class CallState
  {
    Idle,
    Incoming,
    Outgoing,
    Connecting,
    Connected
  };

  void createPeerConnection();
  void attachAudioTrack();
  void attachVideoTrack();
  void cleanupCall();

private:
  Transport* mTransport = nullptr;

  QUuid mCurrentCallId;
  bool mVideoEnabled = false;

  CallState mState = CallState::Idle;

  rtc::Configuration mConfiguration;

  std::shared_ptr<rtc::PeerConnection> mPeerConnection;

  std::shared_ptr<rtc::Track> mAudioTrack;
  std::shared_ptr<rtc::Track> mVideoTrack;

  std::shared_ptr<rtc::RtpPacketizationConfig> mAudioRtpConfig;
  std::shared_ptr<rtc::RtpPacketizationConfig> mVideoRtpConfig;
};