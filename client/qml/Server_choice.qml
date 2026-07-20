import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml 2.15

Item {
    id: root
    property var style: Style{} // Подключаем стиль из Style.qml
    property var colors: style.colors // Подключаем палитру из Style.qml
    property var textStyles: style.textStyles // Подключаем стили текста из Style.qml

    property int maxVisibleServerButtons: 4
    property int serverButtonHeight: 40
    property int serverButtonSpacing: 10

    property bool isEditingServer: false
    property int editingServerIndex: -1


    function openServerDialog(mode, serverName, serverUrl, targetIndex) {
        addServerDialog.isEditing = mode === "edit"
        addServerDialog.editingServerIndex = targetIndex
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

        if (addServerDialog.isEditing && addServerDialog.editingServerIndex >= 0) {
            clientController.updateServer(addServerDialog.editingServerIndex, name, url)
        } else {
            clientController.addServer(name, url)
        }

        clearServerDialogFields()
    }
    
    // Background rectangle to fill the entire area of the parent
    Rectangle {
        anchors.fill: parent
        color: colors.bg_canvas_default
    }

    Column {
        anchors.centerIn: parent
        spacing: 18

        // Logo image at the top
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

        Component.onCompleted: clientController.loadServersFromCsv() // Load servers when the component is completed

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

                Repeater {
                    model: clientController.serversList
                    delegate: ServerButton {
                        serverName: modelData.serverName
                        serverUrl: modelData.serverUrl
                        isOnline: modelData.isOnline
                        onlineText: {
                            if (modelData.isOnline !== false && modelData.onlineCount !== undefined && modelData.onlineCount !== -1) {
                                return modelData.onlineCount + "/" + modelData.usersCount
                            }
                            return ""
                        }
                            
                        onClicked: {
                            root.StackView.view.push("qrc:/Main/qml/Login_server.qml", {
                                "serverName": modelData.serverName,
                                "serverUrl": modelData.serverUrl
                            })
                            clientController.setSelectedServer(modelData) // Set the selected server in the controller
                        }
                        onEditRequested: openServerDialog("edit", serverName, serverUrl, index)
                        onDeleteRequested: clientController.removeServer(index)
                    }
                }

                ServerButton {
                    serverName: "Добавить сервер"
                    isAddButton: true
                    onClicked: openServerDialog("add", "", "", -1)
                }
            }
        }

        // Exit button at the bottom
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
        property int editingServerIndex: -1
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
