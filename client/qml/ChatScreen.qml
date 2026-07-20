import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: chatRoot
    color: "#ffffff" // Фон области чата

    // Свойства, которые мы меняем из главного окна
    property string currentUserName: ""
    property int currentUserId: -1

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
            color: "#f8f9fa" // Подстройте под header background
            
            Rectangle { // Разделительная линия снизу
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#e0e0e0"
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15

                Text {
                    // Переключается автоматически благодаря свойству currentUserName
                    text: chatRoot.currentUserName !== "" ? chatRoot.currentUserName : "Выберите чат..."
                    font.pixelSize: 16
                    font.bold: true
                }
                
                Item { Layout.fillWidth: true } // Распорка
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
                spacing: 10
                
                // В будущем привяжете к модели сообщений из C++: model: chatController.messagesModel
                model: 3 
                
                delegate: Rectangle {
                    width: chatListView.width * 0.7
                    height: 40
                    color: index % 2 === 0 ? "#e3f2fd" : "#f5f5f5"
                    radius: 8
                    anchors.horizontalCenter: index % 2 === 0 ? undefined : parent.horizontalCenter
                    anchors.right: index % 2 === 0 ? parent.right : undefined
                    
                    Text {
                        text: "Пример сообщения номер " + (index + 1)
                        anchors.centerIn: parent
                    }
                }
            }
        }

        // 3. Панель ввода сообщения
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#f8f9fa"

            Rectangle { // Разделительная линия сверху
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: "#e0e0e0"
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                TextField {
                    id: messageField
                    Layout.fillWidth: true
                    placeholderText: "Напишите сообщение..."
                    enabled: chatRoot.currentUserId !== -1 // Блокируем, если чат не выбран
                }

                Button {
                    text: "Отправить"
                    enabled: messageField.text.trim().length > 0 && chatRoot.currentUserId !== -1
                    onClicked: {
                        console.log("Отправка сообщения пользователю", chatRoot.currentUserId, ":", messageField.text)
                        messageField.clear()
                    }
                }
            }
        }
    }
}
