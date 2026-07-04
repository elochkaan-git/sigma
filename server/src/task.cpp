#include "task.h"
#include "command_types.h"
#include <functional>
#include <vector>

Task::Task(std::function<std::vector<Response>()> job)
  : mJob(std::move(job))
{
}

void
Task::run()
{
  // TODO: добавить отлов ошибок выполнения
  std::vector<Response> result = mJob();
  for (const auto r : result) {
    emit responseReady(r);
  }
}
