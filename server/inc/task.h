#pragma once
#include <QObject>
#include <QRunnable>
#include <functional>
#include <vector>
#include "command_types.h"

class Task : public QObject, public QRunnable
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