import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: friendsRoot

    property var style: Style {}
    property var colors: style.colors
    property var textStyles: style.textStyles

    color: colors.bg_canvas_default // Свежий нейтральный фон, подстройте под свои colors.bg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // ================= НАВИГАЦИЯ (ВКЛАДКИ) =================
        RowLayout {
            height: 60
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
        }

        // ================= ОСНОВНОЙ КОНТЕНТ (ЭКРАНЫ) =================
        StackLayout {
            id: mainStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0 // По умолчанию открыт первый экран

            // --- ЭКРАН 1: Список всех друзей ---
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ListView {
                    width: parent.width
                    height: parent.height
                    spacing: 8
                    model: clientController.authHandler.friends

                    Text {
                        anchors.centerIn: parent
                        text: "У вас пока что нет друзей!"
                        visible: parent.count === 0
                        color: "#888888"
                    }
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        color: "white"
                        radius: 8
                        border.color: friendsRoot.colors.border_default

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15
                            
                            // Аватарка (опционально, так как сервер присылает base64)
                            Image {
                                width: 24
                                height: 24
                                fillMode: Image.PreserveAspectCrop 
                                source: modelData.avatar ? "data:image/png;base64," + modelData.avatar : "qrc:/icons/default_avatar.png"
                                visible: modelData.avatar !== ""
                            }

                            Text {
                                text: modelData.login 
                                Layout.fillWidth: true
                                font: friendsRoot.textStyles.userName
                            }
                            
                            Button {
                                text: "Написать"
                                onClicked: {
                                    console.log("Открываем чат с пользователем ID:", modelData.userId)
                                }
                            }
                            
                            Button {
                                text: "Удалить"
                                contentItem: Text {
                                    text: parent.text
                                    color: "red"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                
                                onClicked: {
                                    // Вызываем команду удаления (вам понадобится метод в C++ бэкенде, 
                                    // который отправит wire::RemoveFriend на сервер)
                                    // backend.sendRemoveFriendRequest(modelData.userId)
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
                    width: parent.width
                    height: parent.height
                    spacing: 8
                    model: clientController.authHandler.incomingRequests

                    Text {
                        anchors.centerIn: parent
                        text: "Нет заявок в друзья"
                        visible: parent.count === 0
                        color: "#888888"
                    }

                    delegate: Rectangle {
                            width: parent.width
                            height: 60
                            color: "grey"
                            radius: 8
                            border.color: "#e0e0e0"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 10

                                // Аватарка пользователя
                                Image {
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    sourceSize.width: 24
                                    sourceSize.height: 24
                                    clip: true
                                    fillMode: Image.PreserveAspectCrop 
                                    source: modelData.avatar ? "data:image/png;base64," + modelData.avatar : "qrc:/Main/assets/person.png"
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
                                        // Вызываем метод бэкенда для принятия заявки[cite: 2]
                                        clientController.acceptFriendRequest(modelData.userId);
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
                                        // Вызываем метод бэкенда для отклонения заявки[cite: 2]
                                        clientController.rejectFriendRequest(modelData.userId);
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
                    width: parent.width
                    height: parent.height
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
                        width: parent.width 
                        height: 40
                        color: "white"
                        radius: 8
                        border.color: "#e0e0e0"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15
                            anchors.rightMargin: 15
                            spacing: 10

                            Image {
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                sourceSize.width: 24
                                sourceSize.height: 24
                                clip: true
                                fillMode: Image.PreserveAspectCrop 
                                source: modelData.avatar ? "data:image/png;base64," + modelData.avatar : "qrc:/Main/assets/person.png"
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
            
            // --- ЭКРАН 3: Добавление по ID ---
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
            // Переключаем экран на главный, например
            successToast.show("Запрос в друзья доставлен!");
        }
        function onAcceptFriend() {
            // Переключаем экран на главный, например
            successToast.show("Вы приняли заявку в друзья!");
        }function onRejectFriend() {
            // Переключаем экран на главный, например
            successToast.show("Вы отказали человеку в заявке в друзья");
        }
    }
}
