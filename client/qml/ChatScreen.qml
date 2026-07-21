import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Rectangle {
    id: chatRoot
    

    property var style: Style {}
    property var colors: style.colors
    property var textStyles: style.textStyles
    // Свойства, которые мы меняем из главного окна
    property string currentUserName: ""
    property int currentUserId: -1
    color: chatRoot.colors.bg_canvas_default

    // Слушатель изменений: срабатывает каждый раз, когда меняется собеседник
    onCurrentUserIdChanged: {
        if (currentUserId !== -1) {
            console.log("Загружаем историю чата для пользователя ID:", currentUserId)
            // Здесь будет вызов вашего контроллера:
            // clientController.loadMessagesFor(currentUserId)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Шапка чата (Имя пользователя, с кем общаемся)
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: chatRoot.colors.bg_canvas_default
            
            Rectangle { // Разделительная линия снизу
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: chatRoot.colors.border_muted
            }

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
                    // source: modelData.avatar ? "data:image/png;base64," + modelData.avatar : "qrc:/Main/assets/person.png"
                    source: "qrc:/Main/assets/person.png"
                }

                Text {
                    text: chatRoot.currentUserName
                    Layout.fillWidth: true
                    font: chatRoot.textStyles.userName
                    color: chatRoot.colors.fg_default
                }
                
                // Calls icons here
            }
        }

        // 2. Область сообщений
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: chatListView
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12
                clip: true // Чтобы сообщения не вылезали за пределы списка при прокрутке

                // Автоматический скролл вниз при добавлении новых сообщений
                verticalLayoutDirection: ListView.TopToBottom
                
                model: clientController.chatHandler.currentChatMessages

                delegate: Item {
                    id: delegateRoot
                    
                    // Чередуем: четные — от текущего пользователя (справа), нечетные — от собеседника (слева)
                    readonly property bool isOutgoing: modelData.isOutgoing
                    
                    // Ограничиваем максимальную ширину облачка (70% от ширины списка)
                    width: chatListView.width
                    height: bubbleRectangle.height + 4 // Небольшой отступ для тени

                    Rectangle {
                        id: bubbleRectangle
                        
                        // Ширина подстраивается под текст, но не превышает 70% контейнера
                        width: Math.min(messageText.implicitWidth + 32, chatListView.width * 0.7)
                        height: messageLayout.height + 16
                        
                        // Выравнивание: свои — справа, чужие — слева
                        anchors.right: delegateRoot.isOutgoing ? parent.right : undefined
                        anchors.left: delegateRoot.isOutgoing ? undefined : parent.left

                        // Цветовая схема (используются ваши переменные или запасные фолбэки)
                        color: delegateRoot.isOutgoing 
                            ? (chatRoot.colors.accent_bg ?? "#2B5278") 
                            : (chatRoot.colors.fg_muted ?? "#18222D")
                            
                        // Скругление углов: угол со стороны отправки делается более острым
                        radius: 14
                        Rectangle {
                            // Маленький хак для создания «хвостика» баббла
                            width: 14
                            height: 14
                            color: parent.color
                            anchors.bottom: parent.bottom
                            anchors.right: delegateRoot.isOutgoing ? parent.right : undefined
                            anchors.left: delegateRoot.isOutgoing ? undefined : parent.left
                            visible: true
                        }

                        // Основной контент внутри сообщения
                        Column {
                            id: messageLayout
                            anchors {
                                top: parent.top
                                left: parent.left
                                right: parent.right
                                margins: 8
                                leftMargin: 12
                                rightMargin: 12
                            }
                            spacing: 4

                            Text {
                                id: messageText
                                text: modelData.content
                                width: parent.width
                                wrapMode: Text.Wrap
                                color: delegateRoot.isOutgoing ? "#FFFFFF" : "#F5F5F5"
                                font.pixelSize: 14
                                font.family: "Segoe UI, Roboto, sans-serif"
                            }

                            // Время отправки
                            Text {
                                text: "12:45"
                                anchors.right: parent.right
                                color: delegateRoot.isOutgoing ? "#A1C8EC" : "#7F8C99"
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
        }

        // 3. Панель ввода сообщения
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: chatRoot.colors.bg_canvas_default

            Rectangle { // Разделительная линия сверху
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: chatRoot.colors.border_muted
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                // Выравниваем кнопку по нижнему краю, когда TextArea растет вверх
                Layout.alignment: Qt.AlignBottom 

                ScrollView {
                    Layout.fillWidth: true
                    // Ограничиваем минимальную и максимальную высоту поля
                    implicitHeight: Math.min(messageField.implicitHeight, 120) 
                    Layout.preferredHeight: implicitHeight
                    
                    // Отключаем горизонтальный скроллбар, чтобы текст обязательно переносился
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    TextArea {
                        id: messageField
                        placeholderText: "Напишите сообщение..."
                        enabled: chatRoot.currentUserId !== -1
                        
                        // Включаем автоперенос по словам
                        wrapMode: TextEdit.Wrap

                        // Перехватываем Enter для отправки, а Shift+Enter — для переноса строки
                        Keys.onPressed: (event) => {
                            if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && !(event.modifiers & Qt.ShiftModifier)) {
                                if (sendButton.enabled) {
                                    sendButton.clicked()
                                }
                                event.accepted = true
                            }
                        }
                    }
                }

                Button {
                    id: sendButton
                    text: "Отправить"
                    // Выравниваем кнопку по нижнему краю текстового поля
                    Layout.alignment: Qt.AlignHCenter
                    enabled: messageField.text.trim().length > 0 && chatRoot.currentUserId !== -1
                    onClicked: {
                        console.log("Отправка сообщения пользователю", chatRoot.currentUserId, ":", messageField.text)
                        clientController.chatHandler.saveAndSendMessage(chatRoot.currentUserId, messageField.text)
                        messageField.clear()
                    }
                }
            }
        }
    }
    Toast {
        id: errorToast
    }
    Connections {
        target: clientController.chatHandler // Указываем на конкретный подконтроллер

        // Имя функции формируется автоматически: on + ИмяСигнала с большой буквы
        function onShowErrorToast(message) {
            errorToast.show(message); // Вызываем функцию Toast в UI!
        }
    }
}
