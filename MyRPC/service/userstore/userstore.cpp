#include "../../core/service.h"
#include "userstorehandler.h"

int main(int argc, char *argv[]) {
  UserStoreHandler handler;
  SERVICE.Init(argc, argv);
  SERVICE.RegHandler(&handler);
  SERVICE.Run();
  return 0;
}
