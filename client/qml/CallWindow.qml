import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: callWindowRoot

    // Окно можно свободно перемещать, оно не блокирует главный интерфейс
    width: 380
    height: 540
    minimumWidth: 320
    minimumHeight: 480
    title: "Звонок — " + callerName

    // Делаем окно поверх всех окон, если нужно (по желанию можно убрать)
    // flags: Qt.Window | Qt.WindowStaysOnTopHint

    property var style: Style {}
    property var colors: style.colors ? style.colors : {
        "bg_canvas": "#18222D",
        "fg_default": "#FFFFFF",
        "danger": "#E53935",
        "success": "#4CAF50"
    }

    property string callerName: "Пользователь"
    property int callerId: -1

    // Доступ к CallManager через clientController
    readonly property var callManager: clientController.callManager
    readonly property int currentState: callManager ? callManager.callState : 0
    readonly property bool isVideo: callManager ? callManager.isVideoEnabled : false

    color: colors.bg_canvas

    // Окно автоматически открывается, если звонок в процессе, и закрывается, если Idle (0)
    visible: currentState !== 0

    onClosing: (close) => {
        // Если пользователь жмет на 'X' самого окна ОС во время разговора — сбрасываем звонок
        if (currentState !== 0) {
            clientController.endCallRequest()
        }
    }
    
    Item{
        // --- Переключение UI в зависимости от CallState из C++ ---
        states: [
            State {
                name: "INCOMING"
                when: callWindowRoot.currentState === 1 // CallState::Incoming
                PropertyChanges { target: incomingView; visible: true }
                PropertyChanges { target: outgoingView; visible: false }
                PropertyChanges { target: activeCallView; visible: false }
            },
            State {
                name: "OUTGOING"
                when: callWindowRoot.currentState === 2 // CallState::Outgoing
                PropertyChanges { target: incomingView; visible: false }
                PropertyChanges { target: outgoingView; visible: true }
                PropertyChanges { target: activeCallView; visible: false }
            },
            State {
                name: "CONNECTED"
                when: callWindowRoot.currentState === 3 // CallState::Connected
                PropertyChanges { target: incomingView; visible: false }
                PropertyChanges { target: outgoingView; visible: false }
                PropertyChanges { target: activeCallView; visible: true }
            }
        ]

        // ========================================================
        // 1. РЕЖИМ: Входящий звонок (Принять / Отклонить)
        // ========================================================
        ColumnLayout {
            id: incomingView
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16
            visible: false

            Item { Layout.fillHeight: true }

            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 96
                Layout.preferredHeight: 96
                source: (callerId > 0) ? "image://avatars/" + callerId : "qrc:/Main/assets/person.png"
                fillMode: Image.PreserveAspectCrop
            }

            Text {
                text: callWindowRoot.callerName !== "" ? callWindowRoot.callerName : "Входящий вызов"
                color: colors.fg_default
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: isVideo ? "Входящий видеозвонок..." : "Входящий аудиозвонок..."
                color: "#A0A0A0"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    background: Rectangle { color: colors.danger; radius: 24 }
                    contentItem: Text { text: "Отклонить"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true }
                    onClicked: clientController.rejectCallRequest()
                }

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    background: Rectangle { color: colors.success; radius: 24 }
                    contentItem: Text { text: "Принять"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true }
                    onClicked: clientController.acceptCallRequest()
                }
            }
        }

        // ========================================================
        // 2. РЕЖИМ: Ожидание принятия (Исходящий вызов)
        // ========================================================
        ColumnLayout {
            id: outgoingView
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16
            visible: false

            Item { Layout.fillHeight: true }

            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 96
                Layout.preferredHeight: 96
                source: (callerId > 0) ? "image://avatars/" + callerId : "qrc:/Main/assets/person.png"
                fillMode: Image.PreserveAspectCrop
            }

            Text {
                text: callWindowRoot.callerName
                color: colors.fg_default
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Вызов..."
                color: "#A0A0A0"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                background: Rectangle { color: colors.danger; radius: 24 }
                contentItem: Text { text: "Отмена"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true }
                onClicked: clientController.endCallRequest()
            }
        }

        // ========================================================
        // 3. РЕЖИМ: Разговор (Аудио / Видео поток + Кнопка завершения)
        // ========================================================
        Item {
            id: activeCallView
            anchors.fill: parent
            visible: false

            // Вариант А: Видео звонок
            Rectangle {
                anchors.fill: parent
                color: "#11111B"
                visible: isVideo

                // Экран удаленного участника
                Image {
                    id: remoteVideo
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    source: "image://remoteVideo/"  // провайдер всегда вернёт последний кадр
                }

                // Маленький превью со своей камеры в углу
                Rectangle {
                    width: 100
                    height: 130
                    radius: 8
                    color: "#222"
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    border.width: 1
                    border.color: "#444"
                    clip: true

                    Image {
                        id: localVideo
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        source: "image://localVideo/"
                    }
                }
            }

            // Вариант Б: Аудио звонок
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12
                visible: !isVideo

                Image {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 80
                    source: (callerId > 0) ? "image://avatars/" + callerId : "qrc:/Main/assets/person.png"
                }

                Text {
                    text: callWindowRoot.callerName
                    color: colors.fg_default
                    font.pixelSize: 18
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Разговор идет..."
                    color: colors.success
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // Кнопка завершения вызова внизу окна
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20
                width: 56
                height: 56
                radius: 28
                color: colors.danger

                Button {
                    anchors.fill: parent
                    background: Item {}
                    contentItem: Text {
                        text: "✕"
                        color: "white"
                        font.pixelSize: 22
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: clientController.endCallRequest()
                }
            }
        }
    }
}
