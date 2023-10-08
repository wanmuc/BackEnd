#include "../../core/service.h"
#include "authstorehandler.h"

int main(int argc, char *argv[]) {
  AuthStoreHandler handler;
  SERVICE.Init(argc, argv);
  SERVICE.RegHandler(&handler);
  SERVICE.Run();
  return 0;
}
