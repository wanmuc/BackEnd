#include "../../core/service.h"
#include "authhandler.h"

int main(int argc, char *argv[]) {
  AuthHandler handler;
  SERVICE.Init(argc, argv);
  SERVICE.RegHandler(&handler);
  SERVICE.Run();
  return 0;
}
