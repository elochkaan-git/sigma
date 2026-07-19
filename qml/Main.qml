import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 640
    height: 480
    minimumWidth: 640
    minimumHeight: 480
    title: qsTr("Sigma Messenger")

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: "qrc:/Main/qml/Server_choice.qml"
    }
}
