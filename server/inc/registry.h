#pragma once
#include <QHash>
#include <QReadWriteLock>
#include <QString>
#include <QUuid>

#include <optional>

/**
 * @brief Реестр онлайн пользователей. Предоставляет доступ к словарю вида
 * user_id (из БД) -> client_id (ID сокета)
 *
 */
class OnlineUsersRegistry
{
public:
  /**
   * @brief Регистрирует пользователя в словаре. Так как это не
   * мультисловарь, то невозможна ситуация множественного входа с разных
   * устройств. При попытке входа с другого устройства запрос будет
   * отклонен
   *
   * @param user_id ID пользователя
   * @param client_id ID клиента
   * @return true если пользователь успешно зарегестрирован
   * @return false если пользователь уже залогинен
   */
  bool registerUser(unsigned int user_id, const QUuid& client_id);
  /**
   * @brief Удаляет пользователя из реестра
   *
   * @param user_id ID пользователя
   * @return true если пользователь был удален
   * @return false если такого пользователя нет
   */
  bool removeUser(unsigned int user_id);
  /**
   * @brief Возвращает ID клиента по ID пользователя.
   *
   * @param user_id ID пользователя
   * @return std::optional<QUuid> QUuid, если пользователь есть в реестре, иначе
   * std::nullopt
   */
  std::optional<QUuid> getClientId(unsigned int user_id);
  /**
   * @brief Возвращает число онлайн-пользователей
   *
   * @return unsigned int число онлайн-пользователей
   */
  unsigned int totalOnline();
  std::vector<unsigned int> getOnlineUserIds();

private:
  QHash<unsigned int, QUuid> mOnlineUsers;
  QReadWriteLock mLock;
};