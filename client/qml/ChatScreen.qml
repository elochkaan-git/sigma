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
                Layout.rightMargin: 10
                
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
                IconButton{
                    iconSource: "qrc:/Main/assets/mic.png"
                    Layout.alignment: Qt.AlignRight

                    onClicked: {
                        console.log("Audio phone requested!")
                        clientController.startCallRequest(chatRoot.currentUserId, false)
                    }
                }

                IconButton{
                    iconSource: "qrc:/Main/assets/video-camera.png"
                    Layout.alignment: Qt.AlignRight
                    Layout.rightMargin: 15

                    onClicked: {
                        console.log("Video phone requested!")
                        clientController.startCallRequest(chatRoot.currentUserId, true)
                    }
                }
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

                Loader {
                    id: messageField
                    sourceComponent: style.customTextField
                    Layout.fillWidth: true

                    onLoaded: {
                        item.placeholderText = "Введите сообщение"

                        item.accepted.connect(function() {
                            // Проверяем, активна ли кнопка отправки (прошел ли текст валидацию)
                            if (sendButtonLoader.enabled) {
                                clientController.chatHandler.saveAndSendMessage(
                                    chatRoot.currentUserId, 
                                    messageField.item.text
                                );
                                messageField.item.clear();
                            }
                        })
                    }
                }

                Loader {
                    id: sendButtonLoader
                    sourceComponent: style.customButton
                    Layout.alignment: Qt.AlignHCenter
                    
                    enabled: messageField.item && messageField.item.text.trim().length > 0 && chatRoot.currentUserId !== -1
                    
                    onLoaded: {
                        item.text = "Отправить"
                        item.buttonStyle = "accent"
                        item.enabled = Qt.binding(function() { return sendButtonLoader.enabled })
                        item.clicked.connect(function() {
                            clientController.chatHandler.saveAndSendMessage(
                                chatRoot.currentUserId, 
                                messageField.item.text
                            );
                            // Очищаем текст через загруженный элемент
                            if (messageField.item) {
                                messageField.item.clear(); // или messageField.item.text = ""
                            }
                        })
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
    Connections {
        target: clientController.callManager // Указываем на конкретный подконтроллер

        // Имя функции формируется автоматически: on + ИмяСигнала с большой буквы
        function onShowErrorToast(message) {
            errorToast.show(message); // Вызываем функцию Toast в UI!
        }
    }
}
