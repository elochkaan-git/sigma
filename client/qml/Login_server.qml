// qml/Login_server.qml
import QtQuick
import QtQuick.Controls

Page {
    id: loginPage

    property var style: Style{} // Подключаем стиль из Style.qml
    property var colors: style.colors // Подключаем палитру из Style.qml
    property var textStyles: style.textStyles // Подключаем стили текста из Style.qml
    property string serverName: "Server Name" // Имя сервера, на который осуществляется вход
    property string serverUrl: "Server URL" // URL сервера, на который осуществляется вход

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
            text: "Вход на сервер \"" + serverName + "\""
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
            Row{
                visible: false
                id: forgotPasswordRow
                width: childrenRect.width
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 11
                Text {
                    text: "Забыли пароль?"
                    color: colors.fg_muted
                    font: textStyles.system
                }
                Text {
                    text: "Сброс пароля"
                    color: colors.accent_fg
                    font: textStyles.system
                }
            }
            Row{
                id: registerRow
                width: childrenRect.width
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 11
                Text {
                    text: "Нет аккаунта?"
                    color: colors.fg_muted
                    font: textStyles.system
                }
                Text {
                    text: "Зарегистрироваться"
                    color: colors.accent_fg
                    font: textStyles.system

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            // Здесь можно добавить логику перехода на страницу регистрации
                            loginPage.StackView.view.push("qrc:/Main/qml/Register_server.qml");
                        }
                    }
                }
            }
        }
        Row{
            id: buttonRow
            width: childrenRect.width
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 11

            Loader {
                sourceComponent: style.customButton

                onLoaded: {
                    item.text = "Назад"
                    item.buttonStyle = "dark"
                    item.implicitHeight = 32.5
                    item.clicked.connect(function() {
                        clientController.disconnectFromServer();
                        loginPage.StackView.view.pop();
                    })
                }
            }
            Loader {
                sourceComponent: style.customButton

                onLoaded: {
                    item.text = "Вход"
                    item.buttonStyle = "accent"
                    item.implicitHeight = 32.5
                    item.clicked.connect(function() {
                        // Closing the login page and start main application logic in General_page.qml
                        clientController.loginUser(loginInput.item.text, passwordInput.item.text);
                        
                    })
                }
            }
        }
        
    }

    Toast {
        id: errorToast
    }
    
    Connections {
        target: clientController.authHandler // Указываем на конкретный подконтроллер

        // Имя функции формируется автоматически: on + ИмяСигнала с большой буквы
        function onShowErrorToast(message) {
            errorToast.show(message); // Вызываем функцию Toast в UI!
        }
        
        function onLoginSuccess() {
            var comp = Qt.createComponent("qrc:/Main/qml/General_window.qml");
            if (comp.status === Component.Ready) {
                
                var newWindow = comp.createObject(null);
                if (newWindow) {
                    // New window is created and shown
                    mainWindow.close(); // Close the main window (Main.qml)
                } else {
                    console.error("Could not create General_window.qml");
                }
            } else {
                console.error("Failed to load General_window.qml:", comp.errorString());
            }
        }
    }
    

}
