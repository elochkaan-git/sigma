#pragma once
#include "repositories.h"
#include "statuses.h"
#include <QObject>
#include <QString>

/**
 * @brief Класс-сервис для работы с репозиторием UserRepository в базе данных.
 * 
 * @see UserRepository
 */
class UserService
{
public:
  /**
   * @brief Конструктор сервиса.
   * 
   * @param repo указатель на UserRepository
   */
  UserService(UserRepository* repo);
  /**
   * @brief Метод для регистрации пользователей. Принимает нехэшированный пароль и логин,
   * возвращает статус выполнения операции
   * 
   * @param login логин пользователя
   * @param passwd нехэшированный пароль
   * @return OperationStatus статус операции
   *
   * @see OperationStatus
   */
  OperationStatus registerUser(QString login, QString passwd);

private:
  UserRepository* mRepo;
};