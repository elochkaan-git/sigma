import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 720
    minimumWidth: 640
    minimumHeight: 480
    title: qsTr("Sigma Messenger")

    property var style: Style {}
    property var colors: style.colors
    property var textStyles: style.textStyles

    ListModel {
        id: tabsModel
        ListElement { title: "Аня"; showAvatar: true; showDelete: true; urlAvatar: "https://ui-avatars.com/api/?name=Аня"}
        ListElement { title: "Максим"; showAvatar: true; showDelete: true}
        ListElement { title: "Оля"; showAvatar: true; showDelete: true}
    }

    // Свойство для хранения индекса текущего активного таба
    property int currentTabIndex: 0

    function removeChat(index) {
        if (index < 0 || index >= tabsModel.count)
            return;

        var wasActive = index === root.currentTabIndex;
        tabsModel.remove(index);

        if (root.currentTabIndex > index) {
            root.currentTabIndex -= 1;
        } else if (wasActive && root.currentTabIndex >= tabsModel.count) {
            root.currentTabIndex = Math.max(0, tabsModel.count - 1);
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        // Левая панель (изменяемая)
        Rectangle {
            id: sidePanel
            color: colors.bg_canvas_overlay
            
            // Задаем начальные, минимальные и максимальные размеры
            SplitView.preferredWidth: 200
            SplitView.minimumWidth: 200
            SplitView.maximumWidth: 400

            ButtonGroup { id: tabButtonGroup }

            ColumnLayout {
                width: parent.width
                spacing: 8
                
                // Loader {
                //     sourceComponent: style.chatTabButton
                //     Layout.fillWidth: true
                //     Layout.margins: 12
                //     Layout.bottomMargin: 0
                //     onLoaded: {
                //         item.label: "Друзья"
                //         item.isFriendsButton: false
                //         item.ButtonGroup.group: tabBattonGroup
                //     }
                    
                //     // item.checked: true
                    
                // }
                Text {
                    text: "Личные сообщения"
                    font: root.textStyles.system
                    color: root.colors.fg_muted
                    horizontalAlignment: Text.AlignHLeft
                    Layout.fillWidth: true
                }
                Repeater {
                    model: tabsModel

                    // Загружаем компонент кнопки из нашего файла стилей
                    delegate: Loader {
                        sourceComponent: style.chatTabButton
                        Layout.fillWidth: true
                        Layout.margins: 12
                        Layout.bottomMargin: 0

                        Binding { target: item; property: "label"; value: model.title }
                        Binding { target: item; property: "showRemoveButton"; value: model.showDelete }
                        Binding { target: item; property: "showAvatar"; value: model.showAvatar }
                        Binding { target: item; property: "ButtonGroup.group"; value: tabButtonGroup }
                        Binding { target: item; property: "checked"; value: index === root.currentTabIndex }
                        Binding { target: item; property: "avatarSource"; value: model.urlAvatar || "qrc:/Main/assets/person.png" }

                        Connections {
                            target: item
                            function onCheckedChanged() {
                                if (item.checked) {
                                    root.currentTabIndex = index;
                                }
                            }
                            function onRemoveRequested() {
                                root.removeChat(index);
                            }
                        }
                    }
                }

                // Text {
                //     text: "Версия 1.0.0"
                //     font: root.textStyles.system
                //     color: root.colors.fg_muted
                //     horizontalAlignment: Text.AlignHCenter
                //     Layout.fillWidth: true
                //     Layout.alignment: Qt.AlignBottom
                // }
            }
        }

        // Правая панель (основная область, займет все оставшееся пространство)
        StackLayout {
            id: mainStack
            SplitView.fillWidth: true
            currentIndex: root.currentTabIndex
            
            Rectangle {
                color: "#e3f2fd"
                Text { text: "Содержимое профиля"; anchors.centerIn: parent }
            }
            Rectangle {
                color: "#e8f5e9"
                Text { text: "Экран настроек"; anchors.centerIn: parent }
            }
            Rectangle {
                color: "#fff3e0"
                Text { text: "Панель уведомлений"; anchors.centerIn: parent }
            }
            // initialItem: blueWidget // Какой виджет показать при старте
        }
    }
}
