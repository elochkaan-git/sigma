import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 720
    minimumWidth: 640
    minimumHeight: 480
    title: qsTr("Sigma Messenger")

    property var style: Style {}
    property var colors: style.colors
    property var textStyles: style.textStyles

    property string activeView: "chats"
    property string currentTitle: activeView === "chats" ? "Чаты" : "Друзья"
    property string selectedItem: "Максим"

    property var chatItems: [
        { name: "Максим", preview: "Привет, как дела?", time: "09:41" },
        { name: "Аня", preview: "Сделаем это позже", time: "08:12" },
        { name: "Илья", preview: "Проверил макет", time: "Вчера" }
    ]

    property var friendItems: [
        { name: "Олег", status: "В сети", badge: "●" },
        { name: "Ксюша", status: "Не беспокоить", badge: "◌" },
        { name: "Маша", status: "В сети", badge: "●" }
    ]

    function openView(viewName) {
        activeView = viewName
        currentTitle = viewName === "chats" ? "Чаты" : "Друзья"
        selectedItem = viewName === "chats" ? chatItems[0].name : friendItems[0].name
    }

    Rectangle {
        anchors.fill: parent
        color: colors.bg_canvas_default
    }

    header: ToolBar {
        Rectangle {
            anchors.fill: parent
            color: colors.bg_canvas_overlay
        }
        id: toolbar
        contentHeight: 20
        RowLayout {
            anchors.fill: parent
            spacing: 0

            ToolButton {
                text: "Меню"
                onClicked: console.log("Меню нажато")
            }

            ToolButton {
                text: "Настройки"
                onClicked: console.log("Настройки нажаты")
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                text: "Выход"
                onClicked: Qt.quit()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            width: parent.width
            height: 64
            color: colors.bg_canvas_overlay
            border.color: colors.border_default
            border.width: 1

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 16
                spacing: 12

                Image {
                    width: 32
                    height: 32
                    source: "qrc:/Main/assets/logo.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "Sigma"
                    color: colors.fg_default
                    font: textStyles.header
                }

                Text {
                    text: currentTitle
                    color: colors.fg_muted
                    font: textStyles.messageText
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Text {
            //     anchors.right: parent.right
            //     anchors.verticalCenter: parent.verticalCenter
            //     anchors.rightMargin: 16
            //     text: activeView === "chats" ? "3 новых" : "12 онлайн"
            //     color: colors.accent_fg
            //     font: textStyles.system
            // }
        }

        RowLayout {
            width: parent.width
            height: parent.height
            spacing: 0

            Rectangle {
                width: 150
                height: parent.height
                color: colors.bg_canvas_overlay
                border.color: colors.border_default
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Text {
                        text: "Меню"
                        color: colors.fg_default
                        font: textStyles.userName
                    }

                    Repeater {
                        model: [
                            { key: "friends", label: "Друзья", icon: "👥" },
                            { key: "chats", label: "Чаты", icon: "💬" }
                        ]

                        delegate: Rectangle {
                            width: parent.width
                            height: 42
                            radius: 8
                            color: activeView === modelData.key ? colors.accent_bg_subtle : "transparent"
                            border.color: activeView === modelData.key ? colors.accent_bg : "transparent"
                            border.width: 1

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.openView(modelData.key)
                            }

                            Row {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: 12
                                spacing: 8

                                Text {
                                    text: modelData.icon
                                    font.pointSize: 12
                                    color: colors.fg_default
                                }

                                Text {
                                    text: modelData.label
                                    color: colors.fg_default
                                    font: textStyles.messageText
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: parent.height
                color: colors.bg_canvas_default
                border.color: colors.border_default
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: currentTitle
                        color: colors.fg_default
                        font: textStyles.header
                    }

                    Repeater {
                        model: activeView === "chats" ? chatItems : friendItems

                        delegate: Rectangle {
                            width: parent.width
                            height: 56
                            radius: 10
                            color: selectedItem === modelData.name ? colors.bg_canvas_overlay : colors.bg_canvas_subtle
                            border.color: selectedItem === modelData.name ? colors.accent_bg : colors.border_default
                            border.width: 1

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    selectedItem = modelData.name
                                }
                            }

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 10

                                Rectangle {
                                    width: 40
                                    height: 40
                                    radius: 20
                                    color: colors.accent_bg
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        anchors.centerIn: parent
                                        text: activeView === "chats" ? "C" : "U"
                                        color: colors.fg_on_accent
                                        font: textStyles.userName
                                    }
                                }

                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 2

                                    Text {
                                        text: modelData.name
                                        color: colors.fg_default
                                        font: textStyles.userName
                                    }

                                    Text {
                                        text: activeView === "chats" ? modelData.preview : modelData.status
                                        color: colors.fg_muted
                                        font: textStyles.system
                                    }
                                }

                                Item {
                                    width: 1
                                    height: 1
                                }

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: activeView === "chats" ? modelData.time : modelData.badge
                                    color: colors.fg_muted
                                    font: textStyles.system
                                }
                            }
                        }
                    }
                }
            }
            
        }
    }
}
