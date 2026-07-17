#pragma once
#include "command_types.h"

#include <QObject>
#include <QRunnable>

#include <functional>
#include <vector>

/**
 * @brief Класс-задача, который используется в Dispatch
 * @see Dispatch
 */
class Task
  : public QObject
  , public QRunnable
{
  Q_OBJECT

signals:
  /**
   * @brief Сигнал с ответом, отправляемый после завершения метода run. Может
   * быть испущено несколько сигналов
   *
   * @param result ответ
   * @see server_responses.h
   */
  void responseReady(Response result);

public:
  /**
   * @brief Конструктор класса Task
   *
   * @param job Функция, которая должна выполниться
   * @see Dispatcher::dispatch
   */
  Task(std::function<std::vector<Response>()> job);
  /**
   * @brief Здесь запускается mJob и испускаются сигналы
   *
   */
  void run() override;

private:
  std::function<std::vector<Response>()> mJob;
};