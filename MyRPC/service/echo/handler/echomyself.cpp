#include "../echohandler.h"

int EchoHandler::EchoMySelf(EchoMySelfRequest &request, EchoMySelfResponse &response) {
  response.set_message(request.message());
  return 0;
}
