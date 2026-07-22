import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Button {
    id: control

    // ==========================================
    // 1. СВОЙСТВА И СИГНАЛЫ
    // ==========================================
    // Ссылка на объект темы (передай сюда имя объекта или id твоего файла со стилями)
    property var theme: Style{} // Подключаем стиль из Style.qml
    property var colors: style.colors // Подключаем палитру из Style.qml
    property var textStyles: style.textStyles // Подключаем стили текста из Style.qml

    // Путь к изображению/иконке
    property url iconSource: ""

    // Размер иконки (кнопка будет квадратной с таким размером + padding)
    property real iconSize: 32

    // Стиль кнопки: "accent" (фиолетовая) или "dark" (темная)
    property string buttonStyle: "dark"

    // Размеры кнопки делаются квадратными исходя из iconSize и padding
    implicitWidth: iconSize + leftPadding + rightPadding
    implicitHeight: iconSize + topPadding + bottomPadding

    padding: 6
    

    // ==========================================
    // 2. ИКОНКА (СОДЕРЖИМОЕ)
    // ==========================================
    contentItem: Item {
        implicitWidth: control.iconSize
        implicitHeight: control.iconSize

        Image {
            id: iconImage
            anchors.fill: parent
            source: control.iconSource
            sourceSize.width: control.iconSize
            sourceSize.height: control.iconSize
            fillMode: Image.PreserveAspectFit
            smooth: true
            
            // Прозрачность для состояния disabled (заблокированная кнопка)
            opacity: control.enabled ? 1.0 : 0.4

            Behavior on opacity { 
                NumberAnimation { duration: 150 } 
            }
        }
    }

    // ==========================================
    // 3. ФОН И СТИЛИЗАЦИЯ (Как в твоём customButton)
    // ==========================================
    background: Rectangle {
        radius: 6
        
        color: {
            if (!control.theme) return "#000000" // Резервный цвет, если theme не передан
            if (!control.enabled) return control.theme.colors.bg_canvas_inset

            if (control.buttonStyle === "accent") {
                return control.hovered ? control.theme.colors.accent_bg_hover : control.theme.colors.accent_bg
            } else {
                return control.hovered ? control.theme.colors.bg_canvas_overlay : control.theme.colors.bg_canvas_subtle
            }
        }

        border.width: (control.buttonStyle === "dark" && control.visualFocus) ? 1 : 0
        border.color: control.theme ? control.theme.colors.accent_fg : "transparent"

        Behavior on color { 
            ColorAnimation { duration: 150 } 
        }
    }
}
