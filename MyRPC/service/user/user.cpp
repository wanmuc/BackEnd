#include "../../core/service.h"
#include "userhandler.h"

int main(int argc, char *argv[]) {
  UserHandler handler;
  SERVICE.Init(argc, argv);
  SERVICE.RegHandler(&handler);
  SERVICE.Run();
  return 0;
}
