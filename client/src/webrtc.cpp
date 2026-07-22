#include "webrtc.h"

#include "command_types.h"
#include "commands.h"
#include "logging.h"
#include "responses.h"

#include <rtc/candidate.hpp>
#include <rtc/description.hpp>
#include <rtc/h264rtpdepacketizer.hpp>
#include <rtc/h264rtppacketizer.hpp>
#include <rtc/peerconnection.hpp>
#include <rtc/rtcpnackresponder.hpp>
#include <rtc/rtcpsrreporter.hpp>
#include <rtc/rtpdepacketizer.hpp>
#include <rtc/rtppacketizationconfig.hpp>
#include <rtc/rtppacketizer.hpp>
#include <rtc/track.hpp>

#include <QMetaObject>

namespace {
  // Payload types произвольные, но должны совпадать на обеих сторонах —
  // раз обе стороны это наш собственный клиент, а не браузер, фиксируем их
  // здесь как констранты одного источника правды.
  constexpr int kOpusPayloadType = 111;
  constexpr int kH264PayloadType = 96;
}

static bool
registerMetaTypes()
{
  qRegisterMetaType<Command>("Command");
  return true;
}
static const bool metaTypesRegistered = registerMetaTypes();

WebRtcWrapper::WebRtcWrapper(Transport* transport, QObject* parent)
  : QObject(parent)
  , mTransport(transport)
{
  if (!mTransport) {
    qCritical(appService) << "WebRtcWrapper created with null Transport";
    return;
  }

  connect(this,
          &WebRtcWrapper::requestSendCommand,
          mTransport,
          &Transport::sendCommand);
  mConfiguration.disableAutoNegotiation = true;
}

WebRtcWrapper::~WebRtcWrapper()
{
  cleanupCall();
}

void
WebRtcWrapper::setIceServers(const std::vector<rtc::IceServer>& servers)
{
  if (mPeerConnection) {
    qWarning(appService)
      << "setIceServers() called while a call is active, applies to next call only";
  }
  mConfiguration.iceServers = servers;
}

void
WebRtcWrapper::startCall(unsigned int calleeId, bool withVideo)
{
  if (mState != CallState::Idle) {
    qWarning(appService) << "Unable to start call while another call is active";
    return;
  }
  mVideoEnabled = withVideo;

  createPeerConnection();

  mState = CallState::Outgoing;

  wire::StartCall command;
  command.callee_id = calleeId;
  command.with_video = mVideoEnabled;

  qInfo(appService) << "Starting call to user" << calleeId;

  emit requestSendCommand(command);
}

void
WebRtcWrapper::acceptCall()
{
  if (mState != CallState::Incoming) {
    qWarning(appService) << "No incoming call to accept";
    return;
  }

  createPeerConnection();

  wire::AcceptCall command;
  command.call_id = mCurrentCallId;

  qInfo(appService) << "Accepting call" << mCurrentCallId;

  emit requestSendCommand(command);
}

void
WebRtcWrapper::rejectCall()
{
  if (mState != CallState::Incoming) {
    qWarning(appService) << "No incoming call to reject";
    return;
  }

  wire::RejectCall command;
  command.call_id = mCurrentCallId;

  qInfo(appService) << "Rejecting call" << mCurrentCallId;

  emit requestSendCommand(command);

  cleanupCall();
}

void
WebRtcWrapper::endCall()
{
  if (mCurrentCallId.isNull())
    return;

  wire::EndCall command;
  command.call_id = mCurrentCallId;

  qInfo(appService) << "Ending call" << mCurrentCallId;

  emit requestSendCommand(command);

  cleanupCall();
}

void
WebRtcWrapper::handleStartCallResponse(const wire::StartCallResponse& response)
{
  if (response.status != OperationStatus::OK) {
    qWarning(appService)
      << "Failed to start call, status:" << static_cast<int>(response.status);
    emit callFailed(QString("start_call failed with status %1")
                      .arg(static_cast<int>(response.status)));
    cleanupCall();
    return;
  }

  mCurrentCallId = response.call_id;

  qInfo(appService) << "Call id:" << mCurrentCallId;
}

void
WebRtcWrapper::handleIncomingCall(const wire::IncomingCallResponse& response)
{
  if (mState != CallState::Idle) {
    qWarning(appService) << "Incoming call received while busy";
    emit requestSendCommand(wire::RejectCall{ response.call_id });
    return;
  }

  mCurrentCallId = response.call_id;
  mState = CallState::Incoming;
  mVideoEnabled = response.with_video;

  qInfo(appService) << "Incoming call from" << response.caller_id
                     << "call id:" << response.call_id;
}

void
WebRtcWrapper::handleCallAccepted(const wire::CallAcceptedResponse& response)
{
  if (response.call_id != mCurrentCallId) {
    qWarning(appService) << "Received CallAccepted for unknown call";
    return;
  }

  if (!mPeerConnection) {
    qCritical(appService) << "PeerConnection is not initialized";
    return;
  }

  mState = CallState::Connecting;
  qInfo(appService) << "Remote accepted call. Creating offer";

  try {
    mPeerConnection->setLocalDescription(rtc::Description::Type::Offer);
  } catch (const std::exception& e) {
    qCritical(appService) << "Failed to create local description:" << e.what();
    emit callFailed(QString("Failed to create offer: %1").arg(e.what()));
    cleanupCall();
  }
}

void
WebRtcWrapper::handleCallRejected(const wire::CallRejectedResponse& response)
{
  if (response.call_id != mCurrentCallId) {
    qWarning(appService) << "Received CallRejected for unknown call";
    return;
  }

  qInfo(appService) << "Remote rejected the call";

  cleanupCall();
}

void
WebRtcWrapper::handleCallEnded(const wire::CallEndedResponse& response)
{
  if (response.call_id != mCurrentCallId) {
    qWarning(appService) << "Received CallEnded for unknown call";
    return;
  }

  qInfo(appService) << "Remote ended the call";

  cleanupCall();
}

void
WebRtcWrapper::handleRemoteSdp(const wire::SdpResponse& response)
{
  if (response.call_id != mCurrentCallId) {
    qWarning(appService) << "Received SDP for unknown call";
    return;
  }

  if (!mPeerConnection) {
    qWarning(appService) << "PeerConnection is not initialized";
    return;
  }

  try {
    rtc::Description description(response.sdp.toStdString());
    bool isOffer = description.type() == rtc::Description::Type::Offer;
    qInfo() << "Received remote SDP:" << (isOffer ? "Offer" : "Answer");

    mPeerConnection->setRemoteDescription(description);

    if (!mPeerConnection->localDescription()) {
      qInfo(appService) << "No local description yet, creating it now";
      // Мы получили Offer, значит мы должны ответить Answer
      mPeerConnection->setLocalDescription(rtc::Description::Type::Answer);
    }
  } catch (const std::exception& e) {
    qCritical(appService) << "Failed to process SDP:" << e.what();
    emit callFailed(QString("Failed to process SDP: %1").arg(e.what()));
  }
}

void
WebRtcWrapper::handleRemoteIceCandidate(
  const wire::IceCandidateResponse& response)
{
  if (response.call_id != mCurrentCallId) {
    qWarning(appService) << "Received ICE candidate for unknown call";
    return;
  }

  if (!mPeerConnection) {
    qWarning(appService) << "PeerConnection is not initialized";
    return;
  }

  try {
    rtc::Candidate candidate(response.candidate.toStdString(),
                             response.mid.toStdString());

    mPeerConnection->addRemoteCandidate(candidate);

    qInfo(appService) << "Remote ICE candidate added";
  } catch (const std::exception& e) {
    qCritical(appService) << "Failed to add ICE candidate:" << e.what();
  }
}

void
WebRtcWrapper::handleAcceptCallResponse(const wire::AcceptCallResponse& response)
{
  if (response.call_id != mCurrentCallId) {
    qWarning(appService) << "AcceptCallResponse for unknown call";
    return;
  }

  if (response.status != OperationStatus::OK) {
    qWarning(appService) << "Accept call failed, status:" << static_cast<int>(response.status);
    emit callFailed(QString("Accept call failed with status %1")
                      .arg(static_cast<int>(response.status)));
    cleanupCall();
    return;
  }

  if (!mPeerConnection) {
    qCritical(appService) << "PeerConnection is not initialized";
    return;
  }

  mState = CallState::Connecting;

  qInfo(appService) << "Accept call confirmed. Waiting for remote SDP offer...";
}

void
WebRtcWrapper::handleRejectCallResponse(const wire::RejectCallResponse& response)
{
  if (response.call_id != mCurrentCallId) {
    qWarning(appService) << "RejectCallResponse for unknown call";
    return;
  }

  if (response.status != OperationStatus::OK) {
    qWarning(appService) << "Reject call failed, status:" << static_cast<int>(response.status);
    emit callFailed(QString("Reject call failed with status %1")
                      .arg(static_cast<int>(response.status)));
    cleanupCall();
    return;
  }

  if (!mPeerConnection) {
    qCritical(appService) << "PeerConnection is not initialized";
    return;
  }

  mState = CallState::Connecting;

  qInfo(appService) << "Reject call confirmed";
}


void
WebRtcWrapper::sendAudioFrame(const QByteArray& encodedFrame,
                              uint32_t durationSamples)
{
  if (!mAudioTrack || !mAudioTrack->isOpen() || !mAudioRtpConfig) {
    qWarning(appService) << "Audio track is not ready to send";
    return;
  }

  rtc::binary sample(reinterpret_cast<const std::byte*>(encodedFrame.constData()),
                     reinterpret_cast<const std::byte*>(encodedFrame.constData() +
                                                         encodedFrame.size()));
  mAudioTrack->send(std::move(sample));
  mAudioRtpConfig->timestamp += durationSamples;
}

void
WebRtcWrapper::sendVideoFrame(const QByteArray& encodedFrame,
                              uint32_t durationSamples)
{
  if (!mVideoTrack || !mVideoTrack->isOpen() || !mVideoRtpConfig) {
    qWarning() << "Video track is not ready to send, track:" << (mVideoTrack ? "exists" : "null")
                << "isOpen:" << (mVideoTrack ? mVideoTrack->isOpen() : false)
                << "config:" << (mVideoRtpConfig ? "exists" : "null");
    return;
  }
  qDebug() << "Sending video frame, size:" << encodedFrame.size();

  rtc::binary sample(reinterpret_cast<const std::byte*>(encodedFrame.constData()),
                     reinterpret_cast<const std::byte*>(encodedFrame.constData() +
                                                         encodedFrame.size()));
  mVideoTrack->send(std::move(sample));
  mVideoRtpConfig->timestamp += durationSamples;
}

void
WebRtcWrapper::createPeerConnection()
{
  if (mPeerConnection)
    return;

  qInfo() << "Creating PeerConnection";
  mPeerConnection = std::make_shared<rtc::PeerConnection>(mConfiguration);

  attachAudioTrack(); // всегда добавляем аудио
  if (mVideoEnabled) {
    attachVideoTrack(); // видео только если нужно
  }

  // ВАЖНО: все колбэки ниже libdatachannel вызывает на собственных фоновых
  // потоках, а не на потоке, где живёт WebRtcWrapper/Qt event loop. Поэтому
  // тело каждого колбэка целиком оборачивается в QMetaObject::invokeMethod
  // с Qt::QueuedConnection — иначе чтение/запись mCurrentCallId, mState,
  // mPeerConnection и т.д. из чужого потока — гонка данных.
  mPeerConnection->onLocalDescription([this](rtc::Description description) {
    QString typeStr = (description.type() == rtc::Description::Type::Offer) ? "Offer" : "Answer";
    qInfo(appService) << "Generated local SDP of type:" << typeStr;
    QString sdp = QString::fromStdString(std::string(description));
    QMetaObject::invokeMethod(
      this,
      [this, sdp]() {
        wire::Sdp command;
        command.call_id = mCurrentCallId;
        command.sdp = sdp;

        qInfo(appService) << "Generated local SDP";

        emit requestSendCommand(command);
      },
      Qt::QueuedConnection);
  });

  mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
    QString candidateStr = QString::fromStdString(candidate.candidate());
    QString mid = QString::fromStdString(candidate.mid());
    QMetaObject::invokeMethod(
      this,
      [this, candidateStr, mid]() {
        wire::IceCandidate command;
        command.call_id = mCurrentCallId;
        command.candidate = candidateStr;
        command.mid = mid;

        qInfo(appService) << "Generated ICE candidate";

        emit requestSendCommand(command);
      },
      Qt::QueuedConnection);
  });

  mPeerConnection->onStateChange([this](rtc::PeerConnection::State state) {
    QMetaObject::invokeMethod(
      this,
      [this, state]() {
        qInfo(appService) << "PeerConnection state changed:"
                           << static_cast<int>(state);

        switch (state) {
          case rtc::PeerConnection::State::Connected:
            mState = CallState::Connected;
            emit callEstablished();
            break;

          case rtc::PeerConnection::State::Failed:
            emit callFailed("PeerConnection connection failed");
            cleanupCall();
            break;

          case rtc::PeerConnection::State::Disconnected:
          case rtc::PeerConnection::State::Closed:
            cleanupCall();
            break;

          default:
            break;
        }
      },
      Qt::QueuedConnection);
  });

  mPeerConnection->onGatheringStateChange(
    [this](rtc::PeerConnection::GatheringState state) {
      QMetaObject::invokeMethod(
        this,
        [state]() {
          qInfo(appService) << "ICE gathering state changed:"
                             << static_cast<int>(state);
        },
        Qt::QueuedConnection);
    });
}

void
WebRtcWrapper::attachAudioTrack()
{
  const uint32_t ssrc = 1;

  rtc::Description::Audio audio("audio", rtc::Description::Direction::SendRecv);
  audio.addOpusCodec(kOpusPayloadType);
  audio.addSSRC(ssrc, "audio-send");

  qInfo() << "Attaching video track";
  mAudioTrack = mPeerConnection->addTrack(audio);
  if (mAudioTrack) {
    qInfo() << "Video track created successfully";
  } else {
    qCritical() << "Failed to create video track";
  }

  mAudioRtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(
    ssrc, "audio-send", kOpusPayloadType, rtc::OpusRtpPacketizer::DefaultClockRate);

  auto packetizer = std::make_shared<rtc::OpusRtpPacketizer>(mAudioRtpConfig);
  packetizer->addToChain(std::make_shared<rtc::OpusRtpDepacketizer>());
  packetizer->addToChain(std::make_shared<rtc::RtcpSrReporter>(mAudioRtpConfig));
  packetizer->addToChain(std::make_shared<rtc::RtcpNackResponder>());
  mAudioTrack->setMediaHandler(packetizer);

  mAudioTrack->onFrame([this](rtc::binary data, rtc::FrameInfo) {
    qDebug() << "Audio frame received from remote, size:" << data.size();
    QByteArray bytes(reinterpret_cast<const char*>(data.data()),
                     static_cast<int>(data.size()));
    QMetaObject::invokeMethod(
      this, [this, bytes]() { emit audioFrameReceived(bytes); }, Qt::QueuedConnection);
  });
}

void
WebRtcWrapper::attachVideoTrack()
{
  const uint32_t ssrc = 2;

  rtc::Description::Video video("video", rtc::Description::Direction::SendRecv);
  video.addH264Codec(kH264PayloadType);
  video.addSSRC(ssrc, "video-send");

  qInfo() << "Attaching video track";
  mVideoTrack = mPeerConnection->addTrack(video);
  if (mVideoTrack) {
    qInfo() << "Video track created successfully";
  } else {
    qCritical() << "Failed to create video track";
  }

  mVideoRtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(
    ssrc, "video-send", kH264PayloadType, rtc::H264RtpPacketizer::ClockRate);

  auto packetizer = std::make_shared<rtc::H264RtpPacketizer>(
    rtc::NalUnit::Separator::LongStartSequence, mVideoRtpConfig);
  packetizer->addToChain(
    std::make_shared<rtc::H264RtpDepacketizer>(rtc::NalUnit::Separator::LongStartSequence));
  packetizer->addToChain(std::make_shared<rtc::RtcpSrReporter>(mVideoRtpConfig));
  packetizer->addToChain(std::make_shared<rtc::RtcpNackResponder>());
  mVideoTrack->setMediaHandler(packetizer);

  mVideoTrack->onFrame([this](rtc::binary data, rtc::FrameInfo) {
    qDebug() << "Video frame received from remote, size:" << data.size();
    QByteArray bytes(reinterpret_cast<const char*>(data.data()),
                     static_cast<int>(data.size()));
    QMetaObject::invokeMethod(
      this, [this, bytes]() { emit videoFrameReceived(bytes); }, Qt::QueuedConnection);
  });
}

void
WebRtcWrapper::cleanupCall()
{
  qInfo(appService) << "Cleaning up WebRTC resources";

  const bool wasActive = mPeerConnection != nullptr;

  if (mPeerConnection) {
    try {
      mPeerConnection->close();
    } catch (...) {
    }

    mPeerConnection.reset();
  }

  mAudioTrack.reset();
  mVideoTrack.reset();
  mAudioRtpConfig.reset();
  mVideoRtpConfig.reset();

  mCurrentCallId = QUuid();
  mState = CallState::Idle;
  mVideoEnabled = false;

  if (wasActive) {
    emit callClosed();
  }
}
