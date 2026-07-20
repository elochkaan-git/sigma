#pragma once
#include "wire_command_types.h"
#include "command_types.h"
#include "commands.h"
#include "responses.h"
#include "structures.h"

class CallHandler {
public:
    void handleStartCall(const wire::StartCallResponse& r);
    void handleIncomingCall(const wire::IncomingCallResponse& r); 
    void handleAcceptCall(const wire::AcceptCallResponse& r); 
    void handleCallAccepted(const wire::CallAcceptedResponse& r); 
    void handleRejectCall(const wire::RejectCallResponse& r);
    void handleCallRejected(const wire::CallRejectedResponse& r); 
    void handleEndCall(const wire::EndCallResponse& r); 
    void handleCallEnded(const wire::CallEndedResponse& r); 
    void handleSdp(const wire::SdpResponse& r);
    void handleIceCandidate(const wire::IceCandidateResponse& r);
};
