#pragma once
#include <QHash>
#include <QUuid>
#include <QReadWriteLock>

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
  void deleteRecord(const QUuid& call_id);
  void updateRecord(const QUuid& call_id, CallStatus new_status);
  std::optional<CallRecord> getCallRecord(const QUuid& call_id);

private:
  QHash<QUuid, CallRecord> mCalls;
  QReadWriteLock mLock;
};