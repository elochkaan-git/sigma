#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
  qputenv("QT_MESSAGE_PATTERN", "[%{time h:mm:ss.zzz}] %{type}: %{message}");
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  QObject::connect(
      &engine,
      &QQmlApplicationEngine::objectCreationFailed,
      &app,
      []() { QCoreApplication::exit(-1); },
      Qt::QueuedConnection);

  engine.load("qrc:/Main/qml/Main.qml");
  return app.exec();
}
