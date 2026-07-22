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
            Qt.callLater(function() {
                chatListView.scrollToBottom()
            })
        }
    }

    function formatTimestamp(rawTimestamp) {
        if (!rawTimestamp) return ""
        
        // Если приходит число (ms) или строка ISO/Timestamp
        var date = new Date(rawTimestamp)
        if (isNaN(date.getTime())) {
            // Если пришла строка Unix Timestamp в секундах
            date = new Date(Number(rawTimestamp) * 1000)
        }

        if (isNaN(date.getTime())) return rawTimestamp // Если не удалось распарсить

        var day = String(date.getDate()).padStart(2, '0')
        var month = String(date.getMonth() + 1).padStart(2, '0')
        var hours = String(date.getHours()).padStart(2, '0')
        var minutes = String(date.getMinutes()).padStart(2, '0')

        return `${day}.${month} ${hours}:${minutes}`
    }

    

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Шапка чата (Имя пользователя, с кем общаемся)
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: root.colors.bg_canvas_default
            
            Rectangle { // Разделительная линия снизу
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: root.colors.border_muted
            }

            RowLayout {
                anchors.fill: parent
                spacing: 8
                // anchors.margins: 15
                
                // Аватарка (опционально, так как сервер присылает base64)
                Image {
                    Layout.leftMargin: 10
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    sourceSize.width: 32
                    sourceSize.height: 32
                    clip: true

                    cache: false

                    fillMode: Image.PreserveAspectCrop 
                    source: (chatRoot.currentUserId && chatRoot.currentUserId > 0) 
                        ? "image://avatars/" + chatRoot.currentUserId 
                        : "qrc:/Main/assets/person.png"
                }


                Text {
                    text: (chatRoot.currentUserName) ? chatRoot.currentUserName : ""
                        ? chatRoot.currentUserName 
                        : root.getUserName(chatRoot.currentUserId)
                    Layout.fillWidth: true
                    font: root.textStyles.userName
                    color: root.colors.fg_default
                }
                
                // Calls icons here
            }
        }
        ListView {
            id: chatListView
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 8
            Layout.bottomMargin: 8
            
            // Включаем встроенный скроллбар QtQuick Controls, если нужен
            ScrollBar.vertical: ScrollBar {
                active: true
            }

            // anchors.margins: 15
            spacing: 12
            clip: true // Чтобы сообщения не вылезали за пределы списка при прокрутке

            // Автоматический скролл вниз при добавлении новых сообщений
            verticalLayoutDirection: ListView.TopToBottom
            
            model: clientController.chatHandler.currentChatMessages

            function scrollToBottom() {
                if (count > 0) {
                    positionViewAtIndex(count - 1, ListView.End)
                }
            }
            onCountChanged: {
                Qt.callLater(scrollToBottom)
            }
            onModelChanged: {
                Qt.callLater(scrollToBottom)
            }

            delegate: Item {
                id: delegateRoot
                
                // Чередуем: четные — от текущего пользователя (справа), нечетные — от собеседника (слева)
                readonly property bool isOutgoing: modelData.isOutgoing
                
                // Ограничиваем максимальную ширину облачка (70% от ширины списка)
                width: chatListView.width
                height: bubbleRectangle.height + 6 // Небольшой отступ для тени

                Rectangle {
                    id: bubbleRectangle
                    
                    // Ширина подстраивается под текст, но не превышает 70% контейнера
                    width: Math.min(
                    Math.max(messageText.implicitWidth, timeText.implicitWidth) + 24, 
                    chatListView.width * 0.7
                )
                    height: messageLayout.height + 16
                    
                    // Выравнивание: свои — справа, чужие — слева
                    anchors.right: delegateRoot.isOutgoing ? parent.right : undefined
                    anchors.left: delegateRoot.isOutgoing ? undefined : parent.left

                    // Цветовая схема (используются ваши переменные или запасные фолбэки)
                    color: delegateRoot.isOutgoing 
                        ? (root.colors.accent_bg ?? "#2B5278") 
                        : (root.colors.fg_muted ?? "#18222D")
                        
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
                            color:  "#FFFFFF" 
                            font.pixelSize: 14
                            font.family: "Segoe UI, Roboto, sans-serif"
                        }

                        // Время отправки
                        Text {
                            id: timeText
                            // Вызываем нашу функцию форматирования
                            text: chatRoot.formatTimestamp(modelData.timestamp) 
                            anchors.right: parent.right
                            color: root.colors.fg_on_accent 
                            font.pixelSize: 11
                        }
                    }
                }
                Component.onCompleted: {
                    if (index === chatListView.count) {
                        Qt.callLater(function() {
                            chatListView.positionViewAtIndex(chatListView.count - 1, ListView.End)
                        })
                    }
                }
            }
        }


        // 3. Панель ввода сообщения
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: root.colors.bg_canvas_default

            Rectangle { // Разделительная линия сверху
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: root.colors.border_muted
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
