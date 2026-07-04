#include "task.h"
#include "command_types.h"
#include <functional>

Task::Task(std::function<Response()> job)
  : mJob(std::move(job))
{
}

void
Task::run()
{
  //TODO: добавить отлов ошибок выполнения
  Response result = mJob();
  emit responseReady(result);
}

