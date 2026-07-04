#pragma once
#include "connection_manager.h"
#include "statuses.h"
#include <QString>

/**
 * @brief Класс для непосредственной работы с таблицами в базе данных
 *
 */
class UserRepository
{
public:
  /**
   * @brief Конструктор UserRepository
   *
   * @param manager указатель на ConnectionManager
   * @see ConnectionManager
   */
  UserRepository(ConnectionManager* manager);
  /**
   * @brief Метод регистрации пользователя. Принимает логин и хэшированный
   * пароль. В случае успеха, возвращает ОК, при существовании пользователя
   * возвращает UserExist.
   *
   * @param login логин пользователя
   * @param pwd_hash хэш пароля
   * @return OperationStatus статус выполнения операции
   * @see OperationStatus
   */
  OperationStatus registerUser(QString login, QString pwd_hash);

private:
  ConnectionManager* mConnManager;
};