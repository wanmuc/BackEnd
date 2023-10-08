#include "../../../core/redisclient.hpp"
#include "../userstorehandler.h"

int UserStoreHandler::DeleteUser(DeleteUserRequest &request, DeleteUserResponse &response) {
  std::string key = "user:" + request.user_id();
  std::string error;
  std::string value;
  bool result = Core::RedisClient("backend").Get(key, value, error);
  if (not result) {
    response.set_message("get user failed. " + error);
    return GET_FAILED;
  }
  if (value == "") {
    response.set_message("user not exist");
    return EMPTY_VALUE;
  }
  int64_t delCount = 0;
  result = Core::RedisClient("backend").Del(key, delCount, error);
  if (not result) {
    response.set_message("del user failed. " + error);
    return DEL_FAILED;
  }
  response.set_message("success");
  return 0;
}