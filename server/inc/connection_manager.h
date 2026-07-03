#pragma once
#include <QThreadStorage>
#include <QtSql/QSqlDatabase>
#include <atomic>

/**
 * @brief Структура с параметрами базы данных, необходимых для подключения к этой базе данных.
 */
struct DatabaseInfo
{
  QString hostName = "";
  QString databaseName = "";
  QString userName = "";
  QString password = "";
};

/**
 * @brief Мененджер соединений с базой данных. 
 * 
 * Данный класс необходим для управления соединениями с базой данных. Создается в главном
 * цикле программы единожды, а затем передается в виде указателя во все репозитории.
 *
 * @see UserRepository
 */
class ConnectionManager
{
public:
  /**
   * @brief Конструктор менеджера соединений.
   * 
   * @param db_info Параметры базы данных, к которой нужно подключиться.
   */
  ConnectionManager(const DatabaseInfo& db_info);
  /**
   * @brief Возвращает ссылку на соединение для данного потока.
   * Если в данном потоке еще не было установлено соединение, то оно создается.
   * 
   * @return QSqlDatabase& ссылка на соединение с базой данных.
   */
  QSqlDatabase& currentConnection();

private:
  QThreadStorage<QSqlDatabase> mConnections;
  /// Счётчик для генерации уникальных имён соединений на поток.
  /// Атомарный, так как метод currentConnection() вызывается
  /// из разных воркер-потоков параллельно.
  std::atomic<unsigned int> mThreadCounter{0};
  DatabaseInfo mDatabase;
};