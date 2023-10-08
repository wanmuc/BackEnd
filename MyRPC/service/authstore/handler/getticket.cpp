#include "../../../core/redisclient.hpp"
#include "../authstorehandler.h"

int AuthStoreHandler::GetTicket(GetTicketRequest &request, GetTicketResponse &response) {
  std::string key = "user:ticket:" + request.user_id();
  std::string value;
  std::string error;
  bool result = Core::RedisClient("backend").Get(key, value, error);
  if (not result) {
    response.set_message("failed, " + error);
    return GET_FAILED;
  }
  if (value == "") {
    response.set_message("empty value");
    return EMPTY_VALUE;
  }
  result = Common::Convert::JsonStr2Pb(value, *response.mutable_ticket());
  if (not result) {
    response.set_message("jsonStr2Pb failed");
    return SERIALIZE_FAILED;
  }
  response.set_message("success");
  return 0;
}