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
   * @brief Регистрирует/перезаписывает пользователя в словаре. Так как это не
   * мультисловарь, то невозможна ситуация множественного входа с разных
   * устройств.
   *
   * @param user_id ID пользователя
   * @param client_id ID клиента
   */
  void registerUser(unsigned int user_id, const QUuid& client_id);
  /**
   * @brief Удаляет пользователя из реестра
   *
   * @param user_id ID пользователя
   */
  void removeUser(unsigned int user_id);
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

private:
  QHash<unsigned int, QUuid> mOnlineUsers;
  QReadWriteLock mLock;
};