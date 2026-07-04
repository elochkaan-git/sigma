#pragma once
#include "command_types.h"
#include <QObject>
#include <QRunnable>
#include <functional>
#include <vector>

class Task
  : public QObject
  , public QRunnable
{
  Q_OBJECT

signals:
  void responseReady(Response result);

public:
  Task(std::function<std::vector<Response>()> job);
  void run() override;

private:
  std::function<std::vector<Response>()> mJob;
};