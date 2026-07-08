#include "connection_manager.h"
#include "dispatcher.h"
#include "logging.h"
#include "network_manager.h"
#include "registry.h"
#include "repositories.h"
#include "services.h"
#include <QCoreApplication>
#include <qlogging.h>
#include <stdexcept>

int
main(int argc, char* argv[])
{
  try {
    qSetMessagePattern(
      "[%{time yyyy-MM-dd h:mm:ss.zzz}] [%{type}] [%{category}] "
      "%{message} (%{function})");
    qInfo(app) << "Startup server!";
    QCoreApplication app(argc, argv);

    ConnectionManager conn_manager({ getenv("DB_HOST"),
                                     getenv("DB_NAME"),
                                     getenv("DB_USER"),
                                     getenv("DB_PASSWORD") });
    OnlineUsersRegistry registry;
    UserRepository userRepo(&conn_manager);
    MessageRepository msgRepo(&conn_manager);
    UserService userServive(&userRepo);
    MessageService msgService(&msgRepo);
    Dispatcher dispatcher({ &userServive, &msgService }, &registry);
    NetworkManager net_manager(&dispatcher, &registry);

    return app.exec();
  } catch (const std::runtime_error& error) {
    exit(255);
  } catch (...) {
  };
}