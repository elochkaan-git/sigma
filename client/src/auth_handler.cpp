#include "auth_handler.h"
#include "QDebug"

void AuthHandler::handleRegister(const wire::RegisterUserResponse &r)
{
    if (r.status == OperationStatus::OK) {
        qDebug() << "Success!";
        emit registerSuccess();
    } else {
        if(r.status == OperationStatus::UserExist){
            emit showErrorToast("User with this login exists!");
        } else {
            emit showErrorToast("Internal server error");
        }
    }
}

void AuthHandler::handleLogin(const wire::LoginUserResponse &r)
{
    if (r.status == OperationStatus::OK) {
        m_loggedIn = true;
        m_userId = r.user_id;
        emit loginSuccess(m_userId);
    } else {
        if(r.status == OperationStatus::UserNotExist){
            emit showErrorToast("User with this login doesnt exist!");
        } else if(r.status == OperationStatus::InvalidCredentials) {
            emit showErrorToast("Invalid username or password!");
        } else {
            emit showErrorToast("Internal server error");
        }
    }
}

void AuthHandler::handleSendFriendRequest(const wire::SendFriendRequestResponse &r)
{
    if(r.status == OperationStatus::OK){
        emit sendFriendRequestSuccess();
    } else {
        if(r.status == OperationStatus::RelationAlreadyExist){
            emit showErrorToast("Заявка в друзья к этому пользователю уже есть или вы уже друзья");
        } else if (r.status == OperationStatus::RelationWithYourself){
            emit showErrorToast("Вы не можете добавить самого в себя в друзья");
        } else {
            emit showErrorToast("Внутренняя ошибка сервера");
        }
    }
}

void AuthHandler::handleAcceptFriendRequest(const wire::AcceptFriendRequestResponse &r)
{
    if(r.status == OperationStatus::OK){
        emit acceptFriend();
    } else {
        emit showErrorToast("Произошла не предвиденная ошибка при принятии запроса в друзья");
    }
}

void AuthHandler::handleRejectFriendRequest(const wire::RejectFriendRequestResponse &r)
{
    if(r.status == OperationStatus::OK){
        emit rejectFriend();
    } else {
        emit showErrorToast("Произошла не предвиденная ошибка при отказе запроса в друзья");
    }
}

void AuthHandler::handleRemoveFriend(const wire::RemoveFriendResponse &r)
{
    if(r.status == OperationStatus::OK){
        emit friendDelete();
    } else if(r.status == OperationStatus::UserNotInFriends){
        emit showErrorToast("Пользователь не является вашим другом!");
    } else {
        emit showErrorToast("Произошла не предвиденная ошибка при удалении друга!");
    }
}

void AuthHandler::handleSetAvatar(const wire::SetAvatarResponse &r)
{
    if(r.status == OperationStatus::OK){
        emit setAvatarSuccess();
    } else if(r.status == OperationStatus::InvalidAvatar){
        emit showErrorToast("Не валидное изображени либо размер больше 1MB");
    } else {
        emit showErrorToast("Внутренняя ошибка при отправке аватарки");
    }
}

QVariantList AuthHandler::convertUserList(const std::optional<std::vector<User>>& userVector)
{
    QVariantList result;
    
    // Проверяем и наличие optional, и не пуст ли сам вектор внутри
    if (!userVector.has_value() || userVector->empty()) {
        return result;
    }

    for (const auto& user : userVector.value()) {

        if (m_avatarProvider && !user.avatar.isEmpty()) {
            m_avatarProvider->updateAvatar(QString::number(user.user_id), user.avatar);
        }

        QVariantMap userMap;
        userMap["userId"] = user.user_id; 
        userMap["login"] = user.login; 
        userMap["lastSeen"] = user.last_seen; 
        result.append(userMap);
    }
    
    return result;
}

void AuthHandler::handleGetFriends(const wire::GetFriendsResponse& r)
{
    if (r.status == OperationStatus::OK) {
        m_friends = convertUserList(r.friends);
        emit friendsChanged();
    } else {
        emit showErrorToast("Не удалось загрузить список друзей");
    }
}

void AuthHandler::handleGetFriendRequests(const wire::GetFriendRequestsResponse& r)
{   
    if (r.status == OperationStatus::OK) {
        m_incomingRequests = convertUserList(r.requests);
        emit incomingRequestsChanged();
    } else {
        emit showErrorToast("Не удалось загрузить входящие запросы");
    }
}

void AuthHandler::handleGetSentFriendRequests(const wire::GetSentFriendRequestsResponse& r)
{
    qDebug() << "Friends send get handler!";
    if (r.status == OperationStatus::OK) {
        m_outgoingRequests = convertUserList(r.sent_requests);
        emit outgoingRequestsChanged();
    } else {
        emit showErrorToast("Не удалось загрузить исходящие запросы");
    }
}

void AuthHandler::handleGetOnlineUsers(const wire::GetOnlineUsersResponse &r)
{
    if (r.status == OperationStatus::OK) {
        m_onlineUsers = convertUserList(r.users);
        emit onlineUsersChanged();
    } else {
        emit showErrorToast("Не удалось загрузить список онлайна!");
    }
}
