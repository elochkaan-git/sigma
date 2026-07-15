#include "connection_manager.h"
#include "dispatcher.h"
#include "logging.h"
#include "network_manager.h"
#include "registry.h"
#include "repositories.h"
#include "services.h"

#include <QCoreApplication>
#include <QtLogging>
#include <sodium.h>

#include <stdexcept>

int
main(int argc, char* argv[])
{
  try {
    qSetMessagePattern(
      "[%{time yyyy-MM-dd h:mm:ss.zzz}] [%{type}] [%{category}] "
      "%{message} (%{function})");
    qInfo(app) << "Startup server!";
    QCoreApplication application(argc, argv);

    int status = sodium_init();
    if (status) {
      throw std::runtime_error("Can't init sodium library");
    }
    ConnectionManager conn_manager({ getenv("DB_HOST"),
                                     getenv("DB_NAME"),
                                     getenv("DB_USER"),
                                     getenv("DB_PASSWORD") });
    OnlineUsersRegistry registry;
    UserRepository userRepo(&conn_manager);
    MessageRepository msgRepo(&conn_manager);
    RelationRepository relRepo(&conn_manager);
    UserService userServive(&userRepo);
    MessageService msgService(&msgRepo);
    RelationService relService(&relRepo, &userRepo);
    Dispatcher dispatcher({ &userServive, &msgService, &relService },
                          &registry);
    NetworkManager net_manager(&dispatcher, &registry, "config.ini");

    return application.exec();
  } catch (const std::runtime_error& error) {
    qCritical(app) << error.what();
    exit(255);
  } catch (...) {
  };
}