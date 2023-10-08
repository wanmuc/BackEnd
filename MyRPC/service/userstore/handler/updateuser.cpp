#include "../../../core/redisclient.hpp"
#include "../userstorehandler.h"

int UserStoreHandler::UpdateUser(UpdateUserRequest &request, UpdateUserResponse &response) {
  std::string key = "user:" + request.user().user_id();
  std::string error;
  std::string value;
  bool result = Core::RedisClient("backend").Get(key, value, error);
  if (not result) {
    response.set_message("get user failed. " + error);
    return GET_FAILED;
  }
  if (value == "") {
    response.set_message("user not exist");
    return PARAM_INVALID;
  }
  value = "";
  Common::Convert::Pb2JsonStr(request.user(), value);
  result = Core::RedisClient("backend").Set(key, value, 0, error);
  if (not result) {
    response.set_message("set user failed. " + error);
    return SET_FAILED;
  }
  response.set_message("success");
  return 0;
}