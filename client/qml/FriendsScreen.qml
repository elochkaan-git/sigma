import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: friendsRoot

    signal openChatRequested(int userId, string userName)

    property var style: Style {}
    property var colors: style.colors
    property var textStyles: style.textStyles

    color: colors.bg_canvas_default // Свежий нейтральный фон, подстройте под свои colors.bg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // ================= НАВИГАЦИЯ (ВКЛАДКИ) =================
        ScrollView{
            height: 60
            Layout.fillWidth:true
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff

            RowLayout {
                height: parent.avaibleHeight
                Layout.fillWidth: true
                spacing: 10

                Loader {
                    sourceComponent: style.chatTabButton
                    onLoaded: {
                        item.label = "Все"
                        item.isFriendsButton = true
                        item.isActive = Qt.binding(function() { return mainStack.currentIndex === 0 })
                        item.onClicked.connect(function() { mainStack.currentIndex = 0; clientController.updateFriendsInfo() })
                    }
                }
                Loader {
                    sourceComponent: style.chatTabButton
                    onLoaded: {
                        item.label = "Входящие запросы"
                        item.isFriendsButton = true
                        item.isActive = Qt.binding(function() { return mainStack.currentIndex === 1 })
                        item.onClicked.connect(function() { mainStack.currentIndex = 1; ; clientController.updateIncomingFriendsRequests() })
                    }
                }
                Loader {
                    sourceComponent: style.chatTabButton
                    onLoaded: {
                        item.label = "Исходящие запросы"
                        item.isFriendsButton = true
                        item.isActive = Qt.binding(function() { return mainStack.currentIndex === 2 })
                        item.onClicked.connect(function() { mainStack.currentIndex = 2; ; clientController.updateOutcomingFriendsRequests() })
                    }
                }
                Loader {
                    sourceComponent: style.chatTabButton
                    onLoaded: {
                        item.label = "Добавить по ID"
                        item.isFriendsButton = true
                        item.isActive = Qt.binding(function() { return mainStack.currentIndex === 3 })
                        item.onClicked.connect(function() { mainStack.currentIndex = 3 })
                    }
                }
                Loader {
                    sourceComponent: style.chatTabButton
                    onLoaded: {
                        item.label = "Все кто онлайн"
                        item.isFriendsButton = true
                        item.isActive = Qt.binding(function() { return mainStack.currentIndex === 4 })
                        item.onClicked.connect(function() { mainStack.currentIndex = 4; clientController.updateOnlineUsers(); })
                    }
                }
            }
        }
        

        // ================= ОСНОВНОЙ КОНТЕНТ (ЭКРАНЫ) =================
        StackLayout {
            id: mainStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 5 // По умолчанию открыт первый экран

            // --- ЭКРАН 1: Список всех друзей ---
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ListView {
                    id: friendsListView
                    anchors.fill: parent
                    spacing: 8
                    model: clientController.authHandler.friends

                    Text {
                        anchors.centerIn: parent
                        text: "У вас пока что нет друзей!"
                        visible: parent.count === 0
                        color: "#888888"
                    }
                    delegate: Rectangle {
                        width: friendsListView.width
                        height: 60
                        color: "white"
                        radius: 8
                        border.color: colors.border_default

                        RowLayout {
                            anchors.fill: parent
                            spacing: 8
                            // anchors.margins: 15
                            
                            // Аватарка (опционально, так как сервер присылает base64)
                            Image {
                                Layout.leftMargin: 8
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32
                                sourceSize.width: 32
                                sourceSize.height: 32
                                clip: true
                                fillMode: Image.PreserveAspectCrop 

                                cache: false

                                source: (modelData && modelData.userId) ? "image://avatars/" + modelData.userId 
                                    : "qrc:/Main/assets/person.png"
                            }

                            Text {
                                text: modelData.login 
                                Layout.fillWidth: true
                                font: textStyles.userName
                            }
                            
                            Button {
                                text: "Написать"
                                onClicked: {
                                    friendsRoot.openChatRequested(modelData.userId, modelData.login)
                                }
                            }
                            
                            Button {
                                text: "Удалить"
                                Layout.rightMargin: 8
                                contentItem: Text {
                                    text: parent.text
                                    color: "red"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                
                                onClicked: {
                                    clientController.deleteFriend(modelData.userId);
                                    clientController.updateFriendsInfo();
                                }
                            }
                        }
                    }
                }
            }
            // --- ЭКРАН 2: Входящие заявки ---
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ListView {
                    id: incomingListView
                    anchors.fill: parent
                    spacing: 8
                    model: clientController.authHandler.incomingRequests

                    Text {
                        anchors.centerIn: parent
                        text: "Нет заявок в друзья"
                        visible: parent.count === 0
                        color: "#888888"
                    }

                    delegate: Rectangle {
                            width: incomingListView.width
                            height: 60
                            color: "white"
                            radius: 8
                            border.color: "#e0e0e0"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 10

                                // Аватарка пользователя
                                Image {
                                    Layout.preferredWidth: 32
                                    Layout.preferredHeight: 32
                                    sourceSize.width: 32
                                    sourceSize.height: 32
                                    clip: true

                                    cache: false

                                    fillMode: Image.PreserveAspectCrop 
                                    source: (modelData && modelData.userId) 
                                        ? "image://avatars/" + modelData.userId 
                                        : "qrc:/Main/assets/person.png"
                                }

                                Text {
                                    text: modelData.login + " хочет в друзья" // Подставляем логин[cite: 4]
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                }

                                Button {
                                    text: "Принять"
                                    contentItem: Text {
                                        text: parent.text
                                        color: "green"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        clientController.acceptFriendRequest(modelData.userId);
                                        clientController.updateIncomingFriendsRequests();
                                        clientController.updateFriendsInfo();
                                    }
                                }

                                Button {
                                    text: "Отклонить"
                                    contentItem: Text {
                                        text: parent.text
                                        color: "red"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        clientController.rejectFriendRequest(modelData.userId);
                                        clientController.updateIncomingFriendsRequests();
                                    }
                                }
                            }
                        }
            
                }
            }
            // ---- ЭКРАН 3: Входящие только отобразить. Отозвать заявку нельзя
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ListView {
                    id: outgoingListView
                    anchors.fill: parent
                    spacing: 8
                    model: clientController.authHandler.outgoingRequests

                    Text {
                        anchors.centerIn: parent
                        text: "Нет отправленных запросов"
                        visible: parent.count === 0 // У ListView есть удобное свойство count
                        color: "#888888"
                    }

                    delegate: Rectangle {
                        // Внутри ListView мы управляем шириной через width, а не Layout!
                        width: outgoingListView.width
                        height: 60
                        color: "white"
                        radius: 8
                        border.color: "#e0e0e0"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15
                            anchors.rightMargin: 15
                            spacing: 10

                            Image {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32
                                sourceSize.width: 32
                                sourceSize.height: 32
                                clip: true

                                cache: false

                                fillMode: Image.PreserveAspectCrop 
                                source: (modelData && modelData.userId) 
                                    ? "image://avatars/" + modelData.userId 
                                    : "qrc:/Main/assets/person.png"
                            }

                            Text {
                                text: "Запрос отправлен пользователю " + modelData.login
                                Layout.fillWidth: true 
                                font.pixelSize: 14
                                elide: Text.ElideRight 
                            }

                            Text {
                                text: "Ожидает подтверждения"
                                color: "#888888"
                                font.italic: true
                                font.pixelSize: 12
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight 
                            }
                        }
                    }
                }
            }
            
            // --- ЭКРАН 4: Добавление по ID ---
            Item {
                // Обертка-контейнер, чтобы элементы не растягивались на весь экран
                ColumnLayout {
                    width: 400
                    spacing: 15

                    Text {
                        text: "Введите ID пользователя для отправки запроса:"
                        font: friendsRoot.textStyles.header
                        color: friendsRoot.colors.fg_default
                        // textStyle: textStyles.body (если применимо)
                    }
                    Loader {
                        id: userIdInput
                        Layout.fillWidth: true
                        sourceComponent: style.customTextField

                        onLoaded: {
                            item.placeholderText = "1234"
                            item.implicitWidth = parent.width
                            item.implicitHeight = 30
                        }
                    }

                    Loader {
                        sourceComponent: style.customButton

                        onLoaded: {
                            item.text = "Отправить запрос"
                            item.buttonStyle = "accent"
                            item.implicitHeight = 32.5
                            item.clicked.connect(function() {
                                clientController.sendFriendRequest(userIdInput.item.text);
                                clientController.updateOutcomingFriendsRequests();
                            })
                        }
                    }
                }
            }

            // ---- ЭКРАН 5: Список онлайна
            Item {
                // Контейнер под список и заглушку
                Layout.fillWidth: true
                Layout.fillHeight: true

                ListView {
                    id: onlineUsersView
                    anchors.fill: parent
                    spacing: 8
                    clip: true

                    // Привязываем модель
                    model: clientController.authHandler.onlineUsers

                    delegate: Rectangle {
                        // Чтобы элемент не вылезал за границы списка с учетом скроллбара
                        width: onlineUsersView.width 
                        height: 60
                        color: "white"
                        radius: 8
                        border.color: "#e0e0e0"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15
                            anchors.rightMargin: 15
                            spacing: 10

                            Image {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32
                                sourceSize.width: 32
                                sourceSize.height: 32
                                clip: true
                                fillMode: Image.PreserveAspectCrop 
                                
                                cache: false

                                // modelData работает корректно, когда в QVariantList лежат QVariantMap
                                source: (modelData && modelData.userId) 
                                    ? "image://avatars/" + modelData.userId 
                                    : "qrc:/Main/assets/person.png"
                            }

                            Text {
                                text: "Пользователь: " + (modelData.login ?? "")
                                Layout.fillWidth: true 
                                font.pixelSize: 14
                                elide: Text.ElideRight 
                            }

                            Text {
                                text: "ID: " + (modelData.userId ?? "")
                                color: "#888888"
                                font.pixelSize: 12
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight 
                            }
                        }
                    }
                }

                // Текст-заглушка вынесен поверх ListView
                Text {
                    anchors.centerIn: parent
                    text: "Нет активных пользователей"
                    visible: onlineUsersView.count === 0
                    color: "#888888"
                    font.pixelSize: 14
                }
            }
            
            // ---- ЭКРАН 6: заглушка
            Rectangle {
                id: placeholderScreen
                color: root.colors.bg_canvas_default

                Text {
                    anchors.centerIn: parent
                    anchors.margins: 20
                    
                    text: "Выберите меню сверху!"
                    
                    // Оформление текста (подстройте под свой дизайн)
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#888888" // Нейтральный серый цвет для заглушки
                    
                    // Автоматический перенос текста, если окно станет очень узким
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }

    Toast {
        id: errorToast
    }
    Toast {
        id: successToast
        backgroundColor: style.colors.status_success_bg
    }
    Connections {
        target: clientController.authHandler // Указываем на конкретный подконтроллер

        // Имя функции формируется автоматически: on + ИмяСигнала с большой буквы
        function onShowErrorToast(message) {
            errorToast.show(message); // Вызываем функцию Toast в UI!
        }
        
        function onSendFriendRequestSuccess() {
            successToast.show("Запрос в друзья доставлен!");
        }
        function onAcceptFriend() {
            successToast.show("Вы приняли заявку в друзья!");
        }
        function onRejectFriend() {
            successToast.show("Вы отказали человеку в заявке в друзья");
        }
        function onFriendDelete() {
            successToast.show("Вы успешно удалили пользователя из друзей!");
        }
    }
}
