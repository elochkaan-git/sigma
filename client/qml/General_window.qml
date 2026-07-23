import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.platform as Platform

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 720
    minimumWidth: 640
    minimumHeight: 480
    title: qsTr("Sigma Messenger")

    Style {
        id: globalStyle
    }

    // Псевдонимы или ссылки для удобства:
    readonly property alias style: globalStyle
    readonly property alias colors: globalStyle.colors
    readonly property alias textStyles: globalStyle.textStyles


    // Свойство для хранения индекса текущего активного таба
    property int currentTabIndex: 0


    function getUserName(userId, friendsList) {
        if (!userId || Number(userId) <= 0) return "";

        // Берем список или напрямую из свойства
        var friends = (friendsList !== undefined && friendsList !== null) 
                        ? friendsList 
                        : clientController.authHandler.friends;

        // Проверяем наличие длины (работает и для JS Array, и для C++ QVariantList)
        if (friends && typeof friends.length !== "undefined" && friends.length > 0) {
            for (var i = 0; i < friends.length; i++) {
                var f = friends[i];
                
                // Сравниваем ID
                if (f && Number(f.userId) === Number(userId)) {
                    return f.login ? String(f.login) : ("Пользователь #" + userId);
                }
            }
        }

        // То же самое для списка чатов
        var chats = clientController.chatHandler.chatsList;
        if (chats && typeof chats.length !== "undefined" && chats.length > 0) {
            for (var j = 0; j < chats.length; j++) {
                var c = chats[j];
                if (c && Number(c.userId) === Number(userId)) {
                    if (c.login) return String(c.login);
                }
            }
        }

        return "Пользователь #" + userId;
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        // Левая панель (изменяемая)
        Rectangle {
            id: sidePanel
            color: colors.bg_canvas_overlay
            
            // Задаем начальные, минимальные и максимальные размеры
            SplitView.preferredWidth: 200
            SplitView.minimumWidth: 200
            SplitView.maximumWidth: 400

            ScrollView {
                id: sideMenuScroll
                width: parent.width
                clip: true
                anchors.top: parent.top
                anchors.bottom: bottomSection.top
                anchors.bottomMargin: 8 // Отступ между скроллом и текстом
                anchors.horizontalCenter: parent.horizontalCenter

                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.vertical.width: 8
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                ColumnLayout {
                    id: sideMenuColumn
                    spacing: 8
                    width: sideMenuScroll.availableWidth

                    Loader {
                        sourceComponent: style.chatTabButton
                        Layout.fillWidth: true
                        Layout.margins: 12
                        Layout.bottomMargin: 0
                        onLoaded: {
                            item.label = "Друзья"
                            item.isFriendsButton = true
                            item.isActive = true
                            
                            item.isActive = Qt.binding(function() { return mainStack.currentIndex === 0 })
                            item.onClicked.connect(function() {
                                mainStack.currentIndex = 0
                            })
                        }
                    }

                    Text {
                        text: "Личные сообщения"
                        font: root.textStyles.system
                        color: root.colors.fg_muted
                        horizontalAlignment: Text.AlignHLeft
                        Layout.fillWidth: true
                    }

                    Repeater {
                        model: clientController.chatHandler.chatsList

                        delegate: Loader {
                            id: chatItemLoader
                            
                            Layout.fillWidth: true
                            Layout.margins: 12
                            Layout.bottomMargin: 0

                            readonly property int targetUserId: modelData ? modelData.userId : 0

                            sourceComponent: style.chatTabButton

                            Binding {
                                target: chatItemLoader.item
                                property: "label"
                                value: root.getUserName(chatItemLoader.targetUserId, clientController.authHandler.friends)
                            }

                            Binding {
                                target: chatItemLoader.item
                                property: "isActive"
                                value: mainStack.currentIndex === 1 && chatScreen.currentUserId === chatItemLoader.targetUserId
                            }

                            // Чистая ссылка на провайдер без query-параметров!
                            Binding {
                                target: chatItemLoader.item
                                property: "avatarSource"
                                value: {
                                    if (chatItemLoader.targetUserId > 0) {
                                        return "image://avatars/" + chatItemLoader.targetUserId;
                                    }
                                    return "qrc:/Main/assets/person.png";
                                }
                            }

                            Connections {
                                target: chatItemLoader.item

                                function onClicked() {
                                    chatScreen.currentUserId = chatItemLoader.targetUserId
                                    chatScreen.currentUserName = root.getUserName(chatItemLoader.targetUserId, clientController.authHandler.friends)
                                    
                                    clientController.chatHandler.loadChatWithUser(chatItemLoader.targetUserId)
                                    mainStack.currentIndex = 1
                                }

                                function onRemoveRequested() {
                                    confirmHistoryDeleteDiolog.userId = chatItemLoader.targetUserId
                                    confirmHistoryDeleteDiolog.open()
                                }
                            }
                            
                            Connections {
                                target: clientController.authHandler
                                function onFriendsChanged() {
                                    
                                    // Пробегаем по всем элементам Repeater и заставляем Image перечитать провайдер
                                    for (var i = 0; i < sideMenuColumn.children.length; i++) {
                                        var child = sideMenuColumn.children[i];
                                        // Находим наши Loader'ы
                                        if (child && child.item && child.item.avatarSource !== undefined) {
                                            var currentSource = child.item.avatarSource;
                                            if (currentSource && currentSource.indexOf("image://") === 0) {
                                                // Сбрасываем в пустую строку и возвращаем обратно на следующем кадре
                                                child.item.avatarSource = "";
                                                child.item.avatarSource = currentSource;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            Column{
                id: bottomSection
                anchors.bottom: parent.bottom 
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 12
                spacing: 8
                Text {
                    id: userIdText // Добавили id, чтобы ScrollView мог на него ссылаться
                    text: "Ваш id: " + clientController.authHandler.getUserId()
                    font: root.textStyles.system
                    color: root.colors.fg_muted
                    horizontalAlignment: Text.AlignHLeft
                }
                Loader {
                    id: changeAvatarButton
                    anchors.right: parent.right
                    anchors.left: parent.left
                    sourceComponent: style.customButton
                    onLoaded: {
                        item.text = qsTr("Выбрать аватарку")
                        item.buttonStyle = "transparent"
                        item.clicked.connect(function(){
                            avatarFileDialog.open()
                        })
                    }
                }

            }
        }
        /// ...
        // Правая панель (основная область, займет все оставшееся пространство)
        StackLayout {
            id: mainStack
            SplitView.fillWidth: true
            // Индекс будет: 0 — Друзья, 1 — Чат, 2 - заглушка
            currentIndex: 2 

            // Индекс 0: Экран друзей
            FriendsScreen {
                id: friendsScreen
                // Здесь ваша верстка списка друзей
                onOpenChatRequested: function(userId, userName) {
                    // 1. Задаем данные для экрана чата
                    chatScreen.currentUserName = userName
                    chatScreen.currentUserId = userId

                    // 2. Загружаем сообщения через контроллер
                    clientController.chatHandler.loadChatWithUser(userId)

                    // 3. Переключаем интерфейс на чат
                    mainStack.currentIndex = 1
                }
            }

            // Индекс 1: Экран личных сообщений (Чат)
            ChatScreen {
                id: chatScreen
            }

            Rectangle {
                id: placeholderScreen
                color: root.colors.bg_canvas_overlay

                Text {
                    anchors.centerIn: parent
                    anchors.margins: 20
                    
                    text: "Выберите окно в левом меню!"
                    
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

    FileDialog {
        id: avatarFileDialog
        title: qsTr("Выберите аватарку")
        nameFilters: ["Изображения (*.png *.jpg *.jpeg *.webp)"]
        currentFolder: Platform.StandardPaths.writableLocation(Platform.StandardPaths.PicturesLocation)
        
        onAccepted: {
            // fileUrl содержит путь вида "file:///path/to/image.png"
            clientController.setAvatarRequest(selectedFile)
        }
    }

    Dialog {
        id: confirmHistoryDeleteDiolog
        property int userId: -1
        title: "Удаление истории чата"

        anchors.centerIn: parent
        modal: true
        Overlay.modal: Rectangle {
            color: Qt.rgba(0, 0, 0, 0.28)
        }
        footer: Item {
            width: parent ? parent.width : implicitWidth
            implicitHeight: 48

            Row {
                spacing: 12
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 12

                Loader {
                    id: cancelButtonLoader
                    sourceComponent: style.customButton
                    onLoaded: {
                        item.text = qsTr("Отмена")
                        item.buttonStyle = "transparent"
                        item.clicked.connect(confirmHistoryDeleteDiolog.reject)
                    }
                }

                Loader {
                    id: acceptButtonLoader
                    sourceComponent: style.customButton
                    onLoaded: {
                        item.text = qsTr("Удалить")
                        item.buttonStyle = "accent"
                        item.clicked.connect(confirmHistoryDeleteDiolog.accept)
                    }
                }
            }
        }

        // Настройка внешнего вида окна под темную тему
        background: Rectangle {
            color: colors.bg_canvas_overlay
            border.width: 1
            border.color: colors.border_default
            radius: 8
        }

        header: Label {
            text: confirmHistoryDeleteDiolog.title
            font: textStyles.header
            color: colors.fg_default
            horizontalAlignment: Text.AlignHCenter
            padding: 15
        }

        Text{
            text: "Вы уверены что хотите удалить историю общения с этим пользователем?"
            color: colors.fg_default
            font: textStyles.messageText
            padding: 15
        }

        onAccepted: {
            console.log("Удаление истории для пользователя ", confirmHistoryDeleteDiolog.userId)
            clientController.chatHandler.deleteChatHistory(confirmHistoryDeleteDiolog.userId)

            // Если удаленный чат был открыт прямо сейчас — переключаем на заглушку
            if (chatScreen.currentUserId === confirmHistoryDeleteDiolog.userId) {
                mainStack.currentIndex = 2 // Возврат к заглушке "Выберите окно"
            }
        }
        // onRejected: clearServerDialogFields()

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
        
        function onSetAvatarSuccess() {
            successToast.show("Автарка установлена!");
        }
    }
    Connections {
        target: clientController.authHandler
        function onFriendsChanged() {
        }
    }

    Connections {
        target: clientController.chatHandler
        function onChatsListChanged() {
        }
    }

    CallWindow {
        id: callWindow
        // Автоматически подтягивает данные активного собеседника из чата хотя должен из callManager!
        callerId: clientController.callManager.callerId
    
        // Автоматически высчитываем имя, используя готовую функцию root.getUserName
        callerName: root.getUserName(
            clientController.callManager.callerId, 
            clientController.authHandler.friends
        )
    }
}
