import QtQuick 2.15
import QtQuick.Controls 2.15

QtObject {
    id: root

    // ==========================================
    // 1. ЦВЕТОВАЯ ПАЛИТРА
    // ==========================================
    readonly property QtObject colors: QtObject {
        readonly property color bg_canvas_default:   "#0D1117"
        readonly property color bg_canvas_inset:     "#01050A"
        readonly property color bg_canvas_overlay:   "#161A21"
        readonly property color bg_canvas_subtle:    "#22272E"

        readonly property color accent_bg:           "#8957E6"
        readonly property color accent_bg_hover:     "#9C6ADE"
        readonly property color accent_bg_subtle:    "#1D1330"
        readonly property color accent_fg:           "#A472F7"

        readonly property color border_default:      "#30363D"
        readonly property color border_muted:        "#22272E"

        readonly property color fg_default:          "#CAD1D9"
        readonly property color fg_muted:            "#8B949E"
        readonly property color fg_on_accent:        "#FFFFFF"
        readonly property color fg_subtle:           "#495059"

        readonly property color status_success_fg:   "#57D465"
        readonly property color status_success_bg:   "#2EA34F"
        readonly property color status_success_subtle:"#0F2113"

        readonly property color status_danger_bg:    "#D93532"      
        readonly property color status_danger_fg:    "#FF7C73"
        readonly property color status_danger_subtle:"#261717"

        readonly property color status_warning_bg:   "#BF5600"
        readonly property color status_warning_fg:   "#F0883E"
        readonly property color status_warning_subtle: "#211710"
    }

    // ==========================================
    // 2. СТИЛИ ТЕКСТА (Шрифт: Inter)
    // ==========================================
    readonly property QtObject textStyles: QtObject {
        readonly property font header: Qt.font({
            family: "Inter", pointSize: 18, weight: Font.Bold
        })
        readonly property font userName: Qt.font({
            family: "Inter", pointSize: 14, weight: Font.DemiBold // Semibold
        })
        readonly property font messageText: Qt.font({
            family: "Inter", pointSize: 14, weight: Font.Normal // Regular
        })
        readonly property font system: Qt.font({
            family: "Inter", pointSize: 12, weight: Font.Light
        })
    }

    // ==========================================
    // 3. БАЗОВЫЕ ШАБЛОНЫ ДЛЯ КОМПОНЕНТОВ
    // ==========================================
    
    // // Стиль кнопки (Красные/Фиолетовые/Темные состояния из ТЗ)
    property Component customButton: Component {
        Button {
            id: control
            implicitHeight: 32
            leftPadding: 12
            rightPadding: 12
            topPadding: 4
            bottomPadding: 4

            property string buttonStyle: "accent"

            contentItem: Text {
                text: control.text
                font: root.textStyles.messageText
                color: control.buttonStyle === "accent" ? root.colors.fg_on_accent :
                       (!control.enabled ? root.colors.fg_subtle : root.colors.fg_muted)
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle {
                radius: 6
                color: {
                    if (!control.enabled) return root.colors.bg_canvas_inset;
                    if (control.buttonStyle === "accent") {
                        return control.hovered ? root.colors.accent_bg_hover : root.colors.accent_bg;
                    } else {
                        return control.hovered ? root.colors.bg_canvas_overlay : root.colors.bg_canvas_subtle;
                    }
                }
                border.width: control.buttonStyle === "dark" && control.visualFocus ? 1 : 0
                border.color: root.colors.accent_fg
            }
        }
    }

    // // Стиль текстового поля (Поле ввода "Value")
    property Component customTextField: Component {
        TextField {
            id: control
            implicitWidth: 200
            implicitHeight: 30
            color: root.colors.fg_default
            font: root.textStyles.messageText
            selectedTextColor: root.colors.fg_on_accent
            selectionColor: root.colors.accent_bg
            placeholderTextColor: root.colors.fg_subtle
            verticalAlignment: TextInput.AlignVCenter
            leftPadding: 12

            background: Rectangle {
                radius: 6
                color: root.colors.bg_canvas_inset
                border.color: control.activeFocus ? root.colors.accent_bg : root.colors.border_default
                border.width: 1
            }
        }
    }

}
