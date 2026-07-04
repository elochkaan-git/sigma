#include <QCoreApplication>
#include "connection_manager.h"
#include "dispatcher.h"
#include "network_manager.h"
#include "registry.h"
#include "repositories.h"
#include "services.h"

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  ConnectionManager conn_manager({
    "localhost",
    "users_data",
    "sigma",
    "verycoolpassword"
  });
  OnlineUsersRegistry registry;
  UserRepository userRepo(&conn_manager);
  MessageRepository msgRepo(&conn_manager);
  UserService userServive(&userRepo);
  MessageService msgService(&msgRepo);
  Dispatcher dispatcher({&userServive, &msgService}, &registry);
  NetworkManager net_manager(&dispatcher, &registry);

  return app.exec();
}