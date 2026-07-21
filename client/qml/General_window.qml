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

            ScrollView {
                id: sideMenuScroll
                width: parent.width
                clip: true
                anchors.top: parent.top
                anchors.bottom: userIdText.top
                anchors.bottomMargin: 8 // Отступ между скроллом и текстом
                anchors.horizontalCenter: parent.horizontalCenter

                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.vertical.width: 8
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                ColumnLayout {
                    id: sideMenuColumn
                    spacing: 8
                    width: sideMenuScroll.availableWidth

                    Loader {
                        sourceComponent: style.chatTabButton
                        Layout.fillWidth: true
                        Layout.margins: 12
                        Layout.bottomMargin: 0
                        onLoaded: {
                            item.label = "Друзья"
                            item.isFriendsButton = true
                            item.isActive = true
                            
                            item.isActive = Qt.binding(function() { return mainStack.currentIndex === 0 })
                            item.onClicked.connect(function() {
                                mainStack.currentIndex = 0
                            })
                        }
                    }

                    Text {
                        text: "Личные сообщения"
                        font: root.textStyles.system
                        color: root.colors.fg_muted
                        horizontalAlignment: Text.AlignHLeft
                        Layout.fillWidth: true
                    }

                    Repeater {
                        model: clientController.chatHandler.chatsList

                        delegate: Loader {
                            // Указываем Loader'у использовать ваш компонент стиля
                            sourceComponent: style.chatTabButton
                            
                            Layout.fillWidth: true
                            Layout.margins: 12
                            Layout.bottomMargin: 0
                            
                            // Важно: через свойство 'item' мы настраиваем уже загруженную кнопку,
                            // а доступ к данным модели (modelData) получаем напрямую.
                            onLoaded: {
                                item.label = modelData.userName
                                
                                // Если в вашей кнопке есть флаг checked, можно управлять им через индекс
                                item.isActive = Qt.binding(function() {
                                    return mainStack.currentIndex === 1 && chatScreen.currentUserId === modelData.userId
                                })

                                // Навешиваем обработчик клика на загруженную кнопку
                                item.onClicked.connect(function() {
                                    // 1. Задаем данные для экрана чата
                                    chatScreen.currentUserName = modelData.userName
                                    chatScreen.currentUserId = modelData.userId
                                    
                                    // 2. Даем команду контроллеру загрузить сообщения
                                    clientController.chatHandler.loadChatWithUser(modelData.userId)
                                    
                                    // 3. Переключаем StackLayout на индекс 1 (Экран чата)
                                    mainStack.currentIndex = 1
                                })
                            }
                        }
                    }

                    
                }
            }

            Text {
                id: userIdText // Добавили id, чтобы ScrollView мог на него ссылаться
                text: "Ваш id: " + clientController.authHandler.getUserId()
                font: root.textStyles.system
                color: root.colors.fg_muted
                
                // Позиционирование
                anchors.bottom: parent.bottom // Прижимаем к самому низу панели
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 12 // Красивые отступы от краев панели
                
                horizontalAlignment: Text.AlignHLeft
                
                // Так как элемент теперь позиционируется через anchors, а не Layout, 
                // Layout.fillWidth больше не нужен.
            }
        }
        /// ...
        // Правая панель (основная область, займет все оставшееся пространство)
        StackLayout {
            id: mainStack
            SplitView.fillWidth: true
            // Индекс будет: 0 — Друзья, 1 — Чат, 2 - заглушка
            currentIndex: 2 

            // Индекс 0: Экран друзей
            FriendsScreen {
                id: friendsScreen
                // Здесь ваша верстка списка друзей
            }

            // Индекс 1: Экран личных сообщений (Чат)
            ChatScreen {
                id: chatScreen
                // Свойства, которые будут меняться динамически
                property string currentUserName: ""
                property int currentUserId: -1
                
                // Внутри ChatScreen вы будете использовать эти свойства 
                // для загрузки истории сообщений через ваш C++ или JS контроллер
            }

            Rectangle {
                id: placeholderScreen
                color: root.colors.bg_canvas_overlay

                Text {
                    anchors.centerIn: parent
                    anchors.margins: 20
                    
                    text: "Выберите окно в левом меню!"
                    
                    // Оформление текста (подстройте под свой дизайн)
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#888888" // Нейтральный серый цвет для заглушки
                    
                    // Автоматический перенос текста, если окно станет очень узким
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
