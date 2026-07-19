#include "call_registry.h"

#include "logging.h"

#include <QReadLocker>
#include <QUuid>
#include <QWriteLocker>

#include <optional>

QUuid
CallRegistry::createRecord(unsigned int caller_id, unsigned int callee_id)
{
  QWriteLocker locker(&mLock);
  QUuid call_id = QUuid::createUuid();
  qDebug(appCalls) << QString("Creating call %1 for %2 and %3")
                        .arg(call_id.toString())
                        .arg(caller_id)
                        .arg(callee_id);
  mCalls[call_id] = { caller_id, callee_id, CallStatus::Ringing };
  return call_id;
}

bool
CallRegistry::deleteRecord(const QUuid& call_id)
{
  QWriteLocker locker(&mLock);
  qDebug(appCalls) << QString("Deleting call %1").arg(call_id.toString());
  return mCalls.remove(call_id);
}

bool
CallRegistry::forceEndCall(unsigned int user_id)
{
  QWriteLocker locker(&mLock);
  qDebug(appCalls) << QString("Deleting call with %1").arg(user_id);
  return mCalls.removeIf([&](CallRecord& r) {
    return r.caller_id == user_id || r.callee_id == user_id;
  });
}

bool
CallRegistry::updateRecord(const QUuid& call_id, CallStatus new_status)
{
  QWriteLocker locker(&mLock);
  auto it = mCalls.find(call_id);
  if (it != mCalls.end()) {
    it->status = new_status;
    qDebug(appCalls) << QString("Setting status %1 to call %2")
                          .arg((int)new_status)
                          .arg(call_id.toString());
    return true;
  }
  return false;
}

std::optional<CallRecord>
CallRegistry::getCallRecord(const QUuid& call_id)
{
  QReadLocker locker(&mLock);
  auto it = mCalls.find(call_id);
  if (it != mCalls.end()) {
    return it.value();
  }
  return std::nullopt;
}