#include "../../../core/redisclient.hpp"
#include "../userstorehandler.h"

int UserStoreHandler::CreateUser(CreateUserRequest &request, CreateUserResponse &response) {
  std::string key = "user_alloc_id";
  std::string error;
  int64_t userId = 0;
  bool result = Core::RedisClient("backend").Incr(key, userId, error);
  if (not result) {
    response.set_message("alloc user_id failed. " + error);
    return SET_FAILED;
  }
  key = "user:" + std::to_string(userId);
  std::string value;
  request.mutable_user()->set_user_id(std::to_string(userId));
  Common::Convert::Pb2JsonStr(request.user(), value);
  result = Core::RedisClient("backend").Set(key, value, 0, error);
  if (not result) {
    response.set_message("set user failed. " + error);
    return SET_FAILED;
  }
  response.set_user_id(std::to_string(userId));
  response.set_message("success");
  return 0;
}
