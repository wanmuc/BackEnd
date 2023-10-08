#include "../echohandler.h"

int EchoHandler::FastResp(FastRespRequest &request, FastRespResponse &response) {
  TRACE("get FastResp message[%s]", request.ShortDebugString().c_str());
  return 0;
}
