import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id: toast
    
    property var style: Style{} // Подключаем стиль из Style.qml
    property var colors: style.colors // Подключаем палитру из Style.qml
    property var textStyles: style.textStyles // Подключаем стили текста из Style.qml

    // Настройки по умолчанию
    property alias text: label.text
    property int duration: 2500 // Сколько секунд висит (в мс)
    property color backgroundColor: toast.colors.status_danger_bg // Красный по умолчанию (для ошибок)

    // Центрируем по горизонтали внизу экрана (можно переопределить при вызове)
    x: (parent.width - width) / 2
    y: parent.height - height - 50
    
    width: Math.min(label.implicitWidth + 40, parent.width - 40)
    height: label.implicitHeight + 24
    padding: 0
    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose

    // Фон виджета со скругленными углами
    background: Rectangle {
        color: toast.backgroundColor
        radius: 8
        layer.enabled: true // Легкая тень для объема
    }

    // Текст уведомления
    contentItem: Text {
        id: label
        color: toast.colors.fg_on_accent
        font: toast.textStyles.system
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
    }

    // Плавная анимация появления и исчезновения (Opacity)
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 250 }
    }
    
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 250 }
    }

    // Таймер, который закроет окно через N секунд
    Timer {
        id: dismissTimer
        interval: toast.duration
        running: false
        repeat: false
        onTriggered: toast.close()
    }

    // Функция для удобного вызова
    function show(message, customDuration) {
        if (message !== undefined) {
            toast.text = message;
        }
        if (customDuration !== undefined) {
            toast.duration = customDuration;
        }
        
        dismissTimer.restart(); // Сбрасываем таймер, если вызвали повторно
        toast.open();
    }
}
