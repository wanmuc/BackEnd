#include "../echohandler.h"

int EchoHandler::OneWay(OneWayMessage &request, OneWayResponse &response) {
  TRACE("get OneWay message[%s]", request.ShortDebugString().c_str());
  return 0;
}
