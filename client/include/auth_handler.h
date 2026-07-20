#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "wire_command_types.h"
#include "command_types.h"
#include "commands.h"
#include "responses.h"
#include "structures.h"

class AuthHandler : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("AuthHandler is read-only")

    Q_PROPERTY(QVariantList friends READ friends NOTIFY friendsChanged)
    Q_PROPERTY(QVariantList incomingRequests READ incomingRequests NOTIFY incomingRequestsChanged)
    Q_PROPERTY(QVariantList outgoingRequests READ outgoingRequests NOTIFY outgoingRequestsChanged)

public:
    explicit AuthHandler(QObject *parent = nullptr) : QObject(parent) {}
    void handleRegister(const wire::RegisterUserResponse& r);
    void handleLogin(const wire::LoginUserResponse& r);

    void handleGetFriends(const wire::GetFriendsResponse& r);
    void handleGetFriendRequests(const wire::GetFriendRequestsResponse& r);
    void handleGetSentFriendRequests(const wire::GetSentFriendRequestsResponse& r);

    void handleSendFriendRequest(const wire::SendFriendRequestResponse& r);
    void handleAcceptFriendRequest(const wire::AcceptFriendRequestResponse& r);
    void handleRejectFriendRequest(const wire::RejectFriendRequestResponse& r);
    void handleRemoveFriend(const wire::RemoveFriendResponse& r);

    Q_INVOKABLE unsigned int getUserId() { return m_userId; }
    QVariantList friends() const { return m_friends; }
    QVariantList incomingRequests() const { return m_incomingRequests; }
    QVariantList outgoingRequests() const { return m_outgoingRequests; }

signals:
    void registerSuccess();
    void sendFriendRequestSuccess();
    void showErrorToast(const QString &message);
    void loginSuccess();
    void acceptFriend();
    void rejectFriend();

    void friendsChanged();
    void incomingRequestsChanged();
    void outgoingRequestsChanged();

private:
    QVariantList convertUserList(const std::optional<std::vector<User>>& userVector);

    bool m_loggedIn = false;
    unsigned int m_userId;
    QVariantList m_friends;
    QVariantList m_incomingRequests;
    QVariantList m_outgoingRequests;
};
