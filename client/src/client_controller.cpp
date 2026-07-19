#include "client_controller.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QTimer>
#include <QDebug>

namespace {
QString resolveServersCsvPath(const QString &requestedPath)
{
    QStringList candidates;

    const QString trimmedPath = requestedPath.trimmed();
    if (!trimmedPath.isEmpty()) {
        candidates << trimmedPath;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();

    candidates << appDir + "/servers.csv";
    candidates << appDir + "/assets/servers.csv";
    candidates << appDir + "/Main/assets/servers.csv";
    candidates << currentDir + "/servers.csv";
    candidates << currentDir + "/assets/servers.csv";
    candidates << currentDir + "/Main/assets/servers.csv";
    candidates << QStringLiteral(":/assets/servers.csv");
    candidates << QStringLiteral(":/Main/assets/servers.csv");
    candidates << QStringLiteral("qrc:/assets/servers.csv");
    candidates << QStringLiteral("qrc:/Main/assets/servers.csv");

    for (const QString &candidate : candidates) {
        if (candidate.isEmpty()) {
            continue;
        }

        if (candidate.startsWith(":/", Qt::CaseInsensitive) || candidate.startsWith("qrc:/", Qt::CaseInsensitive)) {
            if (QFile::exists(candidate)) {
                return candidate;
            }
            continue;
        }

        const QFileInfo fileInfo(candidate);
        if (fileInfo.exists() && fileInfo.isFile()) {
            return fileInfo.absoluteFilePath();
        }
    }

    return trimmedPath;
}
} // namespace

ClientController::ClientController(QObject *parent):
    QObject(parent),
    m_isConnected(false)
{
    // updateCSV вызывается только при добавлении/удалении/редактировании
    connect(this, &ClientController::serversListChanged, this, &ClientController::updateCSV);

    pingTimer = new QTimer(this);
    connect(pingTimer, &QTimer::timeout, this, &ClientController::pingAllServers);
    // pingTimer->start(2000); // Опрос раз в 2 секунды
}

QVariantList ClientController::loadServersFromCsv(const QString &csvPath)
{
    const QString path = resolveServersCsvPath(csvPath);
    pathToServersCsv = path; // Сохраняем путь к CSV-файлу для последующего обновления
    qDebug() << "Загрузка списка серверов из CSV-файла:" << path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть CSV-файл серверов:" << path;
        return {};
    }

    QTextStream in(&file);
    QVariantList servers;

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        const QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() < 2)
            continue;

        const QString name = parts.at(0).trimmed();
        const QString url = parts.at(1).trimmed();

        QVariantMap serverEntry;
        serverEntry.insert("serverName", name);
        serverEntry.insert("serverUrl", url);
        serverEntry.insert("isOnline", false); // Изначально считаем сервер оффлайн
        serverEntry.insert("usersCount", 0);    // Изначально считаем,
        serverEntry.insert("onlineCount", 0);   // что пользователей нет
        servers.append(serverEntry);
    }

    serversList = servers; // Сохраняем список серверов в свойство класса
    emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился

    return servers;
}

Q_INVOKABLE void ClientController::addServer(const QString &name, const QString &url)
{
    QVariantMap serverEntry;
    serverEntry.insert("serverName", name);
    serverEntry.insert("serverUrl", url);
    serverEntry.insert("isOnline", false); 
    serverEntry.insert("usersCount", 0);    
    serverEntry.insert("onlineCount", 0);     
    serversList.append(serverEntry);

    emit serversListChanged(); // Уведомляем об изменении списка серверов
    emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился

    pingAllServers(); // Пингуем сразу после добавления нового сервера
}

Q_INVOKABLE void ClientController::removeServer(int index)
{
    if (index >= 0 && index < serversList.size()) {
        serversList.removeAt(index);
        emit serversListChanged(); // Уведомляем об изменении списка серверов
        emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился
    }
}

Q_INVOKABLE void ClientController::updateServer(int index, const QString &name, const QString &url)
{
    if (index >= 0 && index < serversList.size()) {
        QVariantMap serverEntry;
        serverEntry.insert("serverName", name);
        serverEntry.insert("serverUrl", url);
        serversList.replace(index, serverEntry);
        emit serversListChanged(); // Уведомляем об изменении списка серверов
        emit serversStatusChanged(); // Уведомляем QML, что список серверов обновился
    }
}

Q_INVOKABLE void ClientController::setSelectedServer(const QVariant &server)
{
    selectedServer = server; 
    QString serverUrl = selectedServer.toMap()["serverUrl"].toString();
    qDebug() << "Selected server URL:" << serverUrl;
    emit serverSelected(serverUrl);
}

Q_INVOKABLE void ClientController::registerUser(const QString &login, const QString &password) {
    qDebug() << "QML запросил регистрацию для:" << login;
    emit registerRequest(login, password); // Перенаправляем запрос в Транспорт
}

Q_INVOKABLE void ClientController::loginUser(const QString &login, const QString &password) {
    qDebug() << "QML запросил вход для:" << login;
    emit loginRequest(login, password); // Перенаправляем запрос в Транспорт
}

void ClientController::updateCSV(){
    if(pathToServersCsv.isEmpty()){
        qWarning() << "Путь к CSV-файлу серверов не установлен.";
        return;
    }

    QFile file(pathToServersCsv);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Не удалось открыть CSV-файл для записи:" << pathToServersCsv;
        return;
    }
    QTextStream out(&file);

    out << "# serverName,serverUrl\n"; // Заголовок CSV-файла
    for (const QVariant &serverVar : serversList) {
        const QVariantMap serverEntry = serverVar.toMap();
        const QString name = serverEntry.value("serverName").toString();
        const QString url = serverEntry.value("serverUrl").toString();
        out << name << "," << url << "\n";
    }
    file.close();
    qDebug() << "Список серверов успешно обновлен в CSV-файле:" << pathToServersCsv;

}

void ClientController::pingAllServers()
{
    qDebug() << "Пинг всех серверов...";
    for (const QVariant &serverVar : serversList) {
        const QVariantMap serverEntry = serverVar.toMap();
        const QString url = serverEntry.value("serverUrl").toString();
        emit pingServer(url);
    }
}

void ClientController::updateServerStatus(const QString &url, bool isOnline, int usersCount, int onlineCount){
    bool modified = false;
    for (int i = 0; i < serversList.size(); ++i) {
        QVariantMap serverEntry = serversList.at(i).toMap();
        if (serverEntry.value("serverUrl").toString() == url) {
            serverEntry["isOnline"] = isOnline;
            serverEntry["usersCount"] = usersCount;   // Общее число пользователей
            serverEntry["onlineCount"] = onlineCount; // Пользователей онлайн

            serversList.replace(i, serverEntry);
            modified = true;
            break;
        }
    }
    if (modified) {
        // Уведомляем QML, что статус обновился (без перезаписи CSV на диске!)
        emit serversStatusChanged(); 
    }
}
