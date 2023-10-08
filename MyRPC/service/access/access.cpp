#include "../../core/service.h"
#include "accesshandler.h"

int main(int argc, char *argv[]) {
  AccessHandler handler;
  SERVICE.Init(argc, argv);
  SERVICE.RegHandler(&handler);
  SERVICE.Run();
  return 0;
}
