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

void
CallRegistry::deleteRecord(const QUuid& call_id)
{
  QWriteLocker locker(&mLock);
  qDebug(appCalls) << QString("Deleting call %1").arg(call_id.toString());
  mCalls.remove(call_id);
}

void
CallRegistry::updateRecord(const QUuid& call_id, CallStatus new_status)
{
  QWriteLocker locker(&mLock);
  if (mCalls.contains(call_id)) {
    mCalls[call_id].status = new_status;
    qDebug(appCalls) << QString("Setting status %1 to call %2")
                                  .arg((int)new_status)
                                  .arg(call_id.toString());
  }
}

std::optional<CallRecord>
CallRegistry::getCallRecord(const QUuid& call_id)
{
  QReadLocker locker(&mLock);
  if (!mCalls.contains(call_id)) {
    return std::nullopt;
  }
  return mCalls.value(call_id);
}