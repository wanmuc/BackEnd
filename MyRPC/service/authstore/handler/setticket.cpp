#include "../../../common/argv.hpp"
#include "../../../core/redisclient.hpp"
#include "../authstorehandler.h"

int AuthStoreHandler::SetTicket(SetTicketRequest &request, SetTicketResponse &response) {
  std::string key = "user:ticket:" + request.ticket().user_id();
  std::string value;
  std::string error;
  Common::Convert::Pb2JsonStr(request.ticket(), value);
  bool result = false;
  int64_t expireTime = request.expire_time();
  result = Core::RedisClient("backend").Set(key, value, expireTime, error);
  TRACE("set key[%s], value[%s], result[%d]", key.c_str(), value.c_str(), result);
  if (not result) {
    response.set_message("failed, " + error);
    return SET_FAILED;
  }
  response.set_message("success");
  return 0;
}
