#pragma once
#include <QHash>
#include <QReadWriteLock>
#include <QUuid>

#include <optional>

enum class CallStatus
{
  Ringing,
  Active
};

struct CallRecord
{
  unsigned int caller_id;
  unsigned int callee_id;
  CallStatus status;
};

class CallRegistry
{
public:
  QUuid createRecord(unsigned int caller_id, unsigned int callee_id);
  bool deleteRecord(const QUuid& call_id);
  bool forceEndCall(unsigned int user_id);
  bool updateRecord(const QUuid& call_id, CallStatus new_status);
  std::optional<CallRecord> getCallRecord(const QUuid& call_id);
  bool isUserInCall(unsigned int user_id);

private:
  QHash<QUuid, CallRecord> mCalls;
  QReadWriteLock mLock;
};