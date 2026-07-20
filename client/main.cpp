#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "client_controller.h" // Подключаем наш контроллер



int main(int argc, char *argv[])
{
    qputenv("QT_MESSAGE_PATTERN", "[%{time h:mm:ss.zzz}] %{type}: %{message}");
    QGuiApplication app(argc, argv);

    // 1. Создаем экземпляр контроллера
    ClientController clientController;

    QQmlApplicationEngine engine;

    // 2. Внедряем его в корневой контекст QML под именем "clientController"
    engine.rootContext()->setContextProperty("clientController", &clientController);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Обратите внимание: стартовать лучше с Main.qml, где у вас StackView с выбором сервера!
    engine.load("qrc:/Main/qml/Main.qml"); 
    return app.exec();
}
