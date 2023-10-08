#include "../../../core/redisclient.hpp"
#include "../userstorehandler.h"

int UserStoreHandler::ReadUser(ReadUserRequest &request, ReadUserResponse &response) {
  std::string key = "user:" + request.user_id();
  std::string error;
  std::string value;
  bool result = Core::RedisClient("backend").Get(key, value, error);
  if (not result) {
    response.set_message("get user failed. " + error);
    return GET_FAILED;
  }
  if (value == "") {
    response.set_message("user not exist.");
    return PARAM_INVALID;
  }
  result = Common::Convert::JsonStr2Pb(value, *response.mutable_user());
  if (not result) {
    TRACE("value[%s]", value.c_str());
    response.set_message("JsonStr2Pb failed. ");
    return PARSE_FAILED;
  }
  response.set_message("success");
  return 0;
}
