// qml/Register_server.qml
import QtQuick
import QtQuick.Controls

Page {
    id: registerPage

    property var style: Style{} // Подключаем стиль из Style.qml
    property var colors: style.colors // Подключаем палитру из Style.qml
    property var textStyles: style.textStyles // Подключаем стили текста из Style.qml
    property string serverName: "Server Name" // Имя сервера, на который осуществляется вход

    Rectangle {
        anchors.fill: parent
        color: colors.bg_canvas_default
    }

    Column {
        anchors.centerIn: parent
        spacing: 40

        Image {
            width: 64
            height: 64
            source: "qrc:/Main/assets/logo.png"
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Регистрация на сервер \"" + serverName + "\""
            color: colors.fg_default
            font: textStyles.header
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Column {
            id: inputColumn
            anchors.horizontalCenter: parent.horizontalCenter
            // anchors.verticalCenter: parent.verticalCenter
            width: 400
            spacing: 8

            Loader {
                id: loginInput
                anchors.horizontalCenter: parent.horizontalCenter
                sourceComponent: style.customTextField

                onLoaded: {
                    item.placeholderText = "Логин/username"
                    item.implicitWidth = parent.width
                    item.implicitHeight = 30
                }
            }
            Loader {
                id: passwordInput
                anchors.horizontalCenter: parent.horizontalCenter
                sourceComponent: style.customTextField

                onLoaded: {
                    item.placeholderText = "Пароль/password"
                    item.implicitWidth = parent.width
                    item.implicitHeight = 30
                    item.echoMode = TextInput.Password
                }
            }
            Loader {
                id: confirmPasswordInput
                anchors.horizontalCenter: parent.horizontalCenter
                sourceComponent: style.customTextField

                onLoaded: {
                    item.placeholderText = "Подтверждение пароля"
                    item.implicitWidth = parent.width
                    item.implicitHeight = 30
                    item.echoMode = TextInput.Password
                }
            }
            Row{
                id: registerRow
                width: childrenRect.width
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 11
                Text {
                    text: "Уже есть аккаунт?"
                    color: colors.fg_muted
                    font: textStyles.system
                }
                Text {
                    text: "Войти"
                    color: colors.accent_fg
                    font: textStyles.system

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            // Здесь можно добавить логику перехода на страницу регистрации
                            registerPage.StackView.view.pop();
                        }
                    }
                }
            }
        }
        Loader {
            sourceComponent: style.customButton
            anchors.horizontalCenter: parent.horizontalCenter

            onLoaded: {
                item.text = "Зарегистрироваться"
                item.buttonStyle = "accent"
                item.implicitHeight = 32.5
                item.clicked.connect(function() {
                    if(passwordInput.item.text !== confirmPasswordInput.item.text) {
                        errorToast.show("Пароли не совпадают!");
                        return;
                    }
                    if(passwordInput.item.text === "" || confirmPasswordInput.item.text === ""){
                        errorToast.show("Логин или пароль не могут быть пустыми!");
                        return;
                    }
                    clientController.registerUser(loginInput.item.text, passwordInput.item.text);
                })
            }
        }
    }

    Toast {
        id: errorToast
    }
    Toast {
        id: successToast
        backgroundColor: style.colors.status_success_bg
    }
    Connections {
        target: clientController.authHandler // Указываем на конкретный подконтроллер

        // Имя функции формируется автоматически: on + ИмяСигнала с большой буквы
        function onShowErrorToast(message) {
            errorToast.show(message); // Вызываем функцию Toast в UI!
        }
        
        function onRegisterSuccess() {
            // Переключаем экран на главный, например
            successToast.show("Now you can login!");
        }
    }

}
