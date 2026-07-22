import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: callWindowRoot

    width: 400
    height: 560
    minimumWidth: 320
    minimumHeight: 480
    title: "Звонок — " + (callerName || "Пользователь")

    property string callerName: ""
    property int callerId: -1

    readonly property var callManager: clientController ? clientController.callManager : null
    readonly property int currentState: callManager ? callManager.callState : 0
    readonly property bool isVideo: callManager ? callManager.isVideoEnabled : false

    visible: currentState !== 0
    onClosing: (close) => {
        if (currentState !== 0) {
            clientController.endCallRequest()
        }
    }

    color: "#1A2332"

    // Компонент круглой аватарки
    Component {
        id: avatarComponent
        Rectangle {
            width: 100
            height: 100
            radius: 50
            clip: true
            color: "#2A3A4A" // цвет фона, если изображение не загрузится
            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                source: (callerId > 0) ? "image://avatars/" + callerId : "qrc:/Main/assets/person.png"
            }
        }
    }

    StackLayout {
        id: stack
        anchors.fill: parent
        anchors.margins: 24
        currentIndex: {
            if (currentState === 1) return 0      // Incoming
            if (currentState === 2) return 1      // Outgoing
            if (currentState === 3) return 2      // Connected
            return -1
        }

        // ------------------------------------------------------------
        // 1. Входящий звонок
        // ------------------------------------------------------------
        ColumnLayout {
            id: incomingView
            spacing: 16

            Loader {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 100
                Layout.preferredHeight: 100
                sourceComponent: avatarComponent
            }

            Text {
                text: callerName || "Входящий вызов"
                color: "#FFFFFF"
                font.pixelSize: 22
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: isVideo ? "Входящий видеозвонок..." : "Входящий аудиозвонок..."
                color: "#AAB8C9"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    background: Rectangle {
                        color: "#E53935"
                        radius: 25
                    }
                    contentItem: Text {
                        text: "Отклонить"
                        color: "white"
                        font.bold: true
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: clientController.rejectCallRequest()
                }

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    background: Rectangle {
                        color: "#43A047"
                        radius: 25
                    }
                    contentItem: Text {
                        text: "Принять"
                        color: "white"
                        font.bold: true
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: clientController.acceptCallRequest()
                }
            }
        }

        // ------------------------------------------------------------
        // 2. Исходящий звонок
        // ------------------------------------------------------------
        ColumnLayout {
            id: outgoingView
            spacing: 16

            Loader {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 100
                Layout.preferredHeight: 100
                sourceComponent: avatarComponent
            }

            Text {
                text: callerName || "Вызов..."
                color: "#FFFFFF"
                font.pixelSize: 22
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Ожидание ответа..."
                color: "#AAB8C9"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                background: Rectangle {
                    color: "#E53935"
                    radius: 25
                }
                contentItem: Text {
                    text: "Отменить"
                    color: "white"
                    font.bold: true
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: clientController.endCallRequest()
            }
        }

        // ------------------------------------------------------------
        // 3. Активный звонок
        // ------------------------------------------------------------
        Item {
            id: activeCallView

            // Видео-режим
            Rectangle {
                anchors.fill: parent
                color: "#0B1119"
                visible: isVideo

                Image {
                    id: remoteVideo
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    cache: false  // отключаем кеширование на всякий случай
                    source: "image://remoteVideo/?" + clientController.remoteVideoVersion
                }

                // Локальное превью
                Rectangle {
                    width: 100
                    height: 140
                    radius: 8
                    color: "#222"
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    border.width: 2
                    border.color: "#555"
                    clip: true

                    Image {
                        id: localVideo
                        width: 100
                        height: 140
                        // ...
                        cache: false
                        source: "image://localVideo/?" + clientController.localVideoVersion
                    }
                }
            }

            // Аудио-режим
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12
                visible: !isVideo

                Loader {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 80
                    sourceComponent: avatarComponent
                }

                Text {
                    text: callerName || "Собеседник"
                    color: "#FFFFFF"
                    font.pixelSize: 20
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Разговор идёт..."
                    color: "#66BB6A"
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // Кнопка завершения
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20
                width: 60
                height: 60
                radius: 30
                color: "#E53935"

                Button {
                    anchors.fill: parent
                    background: Item {}
                    contentItem: Text {
                        text: "✕"
                        color: "white"
                        font.pixelSize: 28
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: clientController.endCallRequest()
                }
            }
        }
    }

    Connections {
        target: callManager
        enabled: callManager !== null
        // можно добавить обработчики по необходимости
    }
}