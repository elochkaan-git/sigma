#pragma once
#include <QObject>
#include <QRunnable>
#include <functional>
#include "command_types.h"

class Task : public QObject, public QRunnable
{
  Q_OBJECT

signals:
  void responseReady(Response result);

public:
  Task(std::function<Response()> job);
  void run() override;

private:
  std::function<Response()> mJob;
};