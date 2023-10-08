#include "../../core/service.h"
#include "echohandler.h"

int main(int argc, char *argv[]) {
  EchoHandler handler;
  SERVICE.Init(argc, argv);
  SERVICE.RegHandler(&handler);
  SERVICE.Run();
  return 0;
}
