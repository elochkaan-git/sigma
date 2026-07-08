// qml/Register_server.qml
import QtQuick
import QtQuick.Controls

Page {
    Button {
        text: "Назад"
        onClicked: StackView.view.pop()
    }
}
