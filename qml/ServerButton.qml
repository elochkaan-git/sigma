// ServerButton.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: parent.width
    height: 40
    radius: 12
    property var style: Style{} // Подключаем стиль из Style.qml
    property var colors: style.colors // Подключаем палитру из Style.qml
    property var textStyles: style.textStyles // Подключаем стили текста из Style.qml
    color: mouseArea.containsPress ? root.colors.bg_canvas_inset : (mouseArea.containsMouse ? root.colors.bg_canvas_subtle : root.colors.bg_canvas_overlay)

    // --- ВНЕШНИЕ СВОЙСТВА (кастомизация под каждый сервер) ---
    property string serverName: "Название сервера"
    property string onlineText: "0/0"
    property string serverUrl: ""
    property bool isOnline: true
    property bool isAddButton: false

    // --- СИГНАЛЫ (чтобы сообщать главному окну о кликах) ---
    signal clicked()
    signal editRequested()
    signal deleteRequested()

    Behavior on color { ColorAnimation { duration: 100 } }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 15

        Text {
            text: root.serverName
            color: root.colors.fg_default
            font: root.textStyles.userName
            Layout.fillWidth: true
            // Если это кнопка добавления, выравниваем текст по центру
            horizontalAlignment: root.isAddButton ? Text.AlignHCenter : Text.AlignLeft
        }

        // Скрываем онлайн, если это кнопка добавления
        Text {
            text: root.onlineText
            color: root.colors.fg_default
            font: root.textStyles.system
            visible: !root.isAddButton
        }

        // Скрываем индикатор, если это кнопка добавления
        Rectangle {
            width: 10
            height: 10
            radius: 6
            color: root.isOnline ? root.colors.status_success_fg : root.colors.status_danger_fg
            Layout.alignment: Qt.AlignVCenter
            visible: !root.isAddButton
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
                root.clicked()
            } else if (mouse.button === Qt.RightButton && !root.isAddButton) {
                var pos = root.mapToItem(root.window ? root.window.contentItem : root.parent, mouse.x, mouse.y)
                contextMenu.popup(pos.x, pos.y)
            }
        }
    }

    Menu {
        id: contextMenu
        parent: root.window ? root.window.contentItem : root.parent

        MenuItem {
            text: qsTr("Изменить сервер")
            onTriggered: root.editRequested()
        }

        MenuItem {
            text: qsTr("Удалить сервер")
            onTriggered: root.deleteRequested()
        }
    }
}
