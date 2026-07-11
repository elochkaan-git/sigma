import QtQuick
import QtQuick.Controls

Item {
    id: root
    
    property var style: Style{} // Подключаем стиль из Style.qml
    property var colors: style.colors // Подключаем палитру из Style.qml
    property var textStyles: style.textStyles // Подключаем стили текста из Style.qml

    property int maxVisibleServerButtons: 4
    property int serverButtonHeight: 40
    property int serverButtonSpacing: 10

    property bool isEditingServer: false
    property var editingServerButton: null

    function openServerDialog(mode, serverName, serverUrl, targetButton) {
        addServerDialog.isEditing = mode === "edit"
        addServerDialog.editingServerButton = targetButton
        addServerDialog.dialogServerName = serverName || ""
        addServerDialog.dialogServerUrl = serverUrl || ""

        if (serverNameInput.item) {
            serverNameInput.item.text = addServerDialog.dialogServerName
        }
        if (serverUrlInput.item) {
            serverUrlInput.item.text = addServerDialog.dialogServerUrl
        }
        if (acceptButtonLoader.item) {
            acceptButtonLoader.item.text = addServerDialog.isEditing ? qsTr("Сохранить") : qsTr("ОК")
        }

        addServerDialog.open()
    }

    function clearServerDialogFields() {
        if (serverNameInput.item) {
            serverNameInput.item.clear()
        }
        if (serverUrlInput.item) {
            serverUrlInput.item.clear()
        }
    }

    function saveServerDialog() {
        var name = serverNameInput.item ? serverNameInput.item.text : ""
        var url = serverUrlInput.item ? serverUrlInput.item.text : ""
        name = name.trim()
        url = url.trim()

        if (!name) {
            return
        }

        if (addServerDialog.isEditing && addServerDialog.editingServerButton) {
            addServerDialog.editingServerButton.serverName = name
            addServerDialog.editingServerButton.serverUrl = url
        } else {
            var component = Qt.createComponent("ServerButton.qml")
            if (component.status === Component.Ready) {
                var button = component.createObject(serverListColumn, {
                    "serverName": name,
                    "serverUrl": url,
                    "onlineText": "0/20",
                    "isOnline": true
                })
                button.onClicked.connect(function() {
                    console.log("Подключаемся к " + button.serverName + "...")
                })
                button.onDeleteRequested.connect(function() {
                    button.destroy()
                })
                button.onEditRequested.connect(function() {
                    openServerDialog("edit", button.serverName, button.serverUrl, button)
                })
            } else {
                console.error(component.errorString())
            }
        }

        clearServerDialogFields()
    }
    
    Rectangle {
        anchors.fill: parent
        color: colors.bg_canvas_default
    }

    Column {
        anchors.centerIn: parent
        spacing: 18

        Image {
            width: 64
            height: 64
            source: "qrc:/Main/assets/logo.png"
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Выбор сервера"
            color: colors.fg_default
            font: textStyles.header
            anchors.horizontalCenter: parent.horizontalCenter
        }

        ScrollView {
            id: serverListScroll
            width: 400
            height: serverListColumn.children.length > maxVisibleServerButtons
                ? maxVisibleServerButtons * serverButtonHeight + (maxVisibleServerButtons - 1) * serverButtonSpacing
                : serverListColumn.implicitHeight
            clip: true
            anchors.horizontalCenter: parent.horizontalCenter
            
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.width: 8

            Column {
                id: serverListColumn
                spacing: serverButtonSpacing
                width: serverListScroll.availableWidth

                ServerButton {
                    serverName: "Основной выживач"
                    serverUrl: "https://main.example"
                    onlineText: "12/20"
                    isOnline: true
                    onClicked: root.StackView.view.push("qrc:/Main/qml/Login_server.qml")
                    onEditRequested: openServerDialog("edit", serverName, serverUrl, this)
                    onDeleteRequested: console.log("Запрос на удаление Основного сервера")
                }

                ServerButton {
                    serverName: "Тестовый сервер"
                    serverUrl: "https://test.example"
                    onlineText: "0/20"
                    isOnline: false
                    onClicked: root.StackView.view.push("qrc:/Main/qml/Register_server.qml")
                    onEditRequested: openServerDialog("edit", serverName, serverUrl, this)
                    onDeleteRequested: console.log("Запрос на удаление Тестового сервера")
                }

                ServerButton {
                    serverName: "Добавить сервер"
                    isAddButton: true
                    onClicked: openServerDialog("add", "", "", null)
                }
            }
        }

        Loader {
            anchors.horizontalCenter: parent.horizontalCenter
            sourceComponent: style.customButton

            onLoaded: {
                item.text = "Выход"
                item.buttonStyle = "accent"
                item.implicitHeight = 32.5
                item.clicked.connect(function() {
                    Qt.quit()
                })
            }
        }
    }
    // --- ДИАЛОГОВОЕ ОКНО ВВОДА ДАННЫХ ---
    Dialog {
        id: addServerDialog
        property bool isEditing: false
        property var editingServerButton: null
        property string dialogServerName: ""
        property string dialogServerUrl: ""
        title: isEditing ? "Изменить сервер" : "Новый сервер"

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
                        item.clicked.connect(addServerDialog.reject)
                    }
                }

                Loader {
                    id: acceptButtonLoader
                    sourceComponent: style.customButton
                    onLoaded: {
                        item.text = addServerDialog.isEditing ? qsTr("Сохранить") : qsTr("ОК")
                        item.buttonStyle = "accent"
                        item.clicked.connect(addServerDialog.accept)
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
            text: addServerDialog.title
            font: textStyles.header
            color: colors.fg_default
            horizontalAlignment: Text.AlignHCenter
            padding: 15
        }

        // Содержимое диалога (поля ввода)
        Column {
            spacing: 15
            width: 300

            Loader {
                id: serverNameInput
                anchors.horizontalCenter: parent.horizontalCenter
                sourceComponent: style.customTextField

                onLoaded: {
                    item.placeholderText = "Имя сервера"
                    item.implicitWidth = parent.width
                    item.implicitHeight = 30
                    item.text = addServerDialog.dialogServerName
                }
            }
            Loader {
                id: serverUrlInput
                anchors.horizontalCenter: parent.horizontalCenter
                sourceComponent: style.customTextField

                onLoaded: {
                    item.placeholderText = "URL или IP:Порт"
                    item.implicitWidth = parent.width
                    item.implicitHeight = 30
                    item.text = addServerDialog.dialogServerUrl
                }
            }
        }

        onAccepted: saveServerDialog()
        onRejected: clearServerDialogFields()

    }
}
