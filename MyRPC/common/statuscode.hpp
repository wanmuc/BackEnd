#pragma once

#include <assert.h>

#include <map>
#include <string>

#include "singleton.hpp"

#define STATUS_CODE Common::Singleton<Common::StatusCode>::Instance()

enum STATUS_CODE_FRAME_DEF {
  SUCCESS = 0,                  // 成功
  SOCKET_CREATE_FAILED = -100,  // socket创建失败
  CONNECTION_FAILED = -101,     // 连接失败
  WRITE_FAILED = -102,          // 写失败
  READ_FAILED = -103,           // 读失败
  NOT_SUPPORT_RPC = -300,       // 不支持的rpc调用
  SERIALIZE_FAILED = -301,      // 序列化失败
  PARSE_FAILED = -302,          // 解析失败
  PARAM_INVALID = -400,         // 参数无效
};

enum STATUS_CODE_REDIS_DEF {
  EMPTY_VALUE = -1000,  // value为空
  GET_FAILED = -1001,   // 获取失败
  SET_FAILED = -1002,   // 设置失败
  DEL_FAILED = -1003,   // 删除失败
  EXEC_FAILED = -1004,  // 执行失败
};

namespace Common {
class StatusCode {
 public:
  StatusCode() {
    Set(SUCCESS, "success");
    Set(SOCKET_CREATE_FAILED, "socket create failed");
    Set(CONNECTION_FAILED, "connection failed");
    Set(WRITE_FAILED, "write failed");
    Set(READ_FAILED, "read failed");
    Set(NOT_SUPPORT_RPC, "not support rpc");
    Set(SERIALIZE_FAILED, "serialize failed");
    Set(PARSE_FAILED, "parse failed");
    Set(PARAM_INVALID, "param invalid");
    Set(EMPTY_VALUE, "empty value");
    Set(GET_FAILED, "get failed");
    Set(SET_FAILED, "set failed");
    Set(DEL_FAILED, "del failed");
    Set(EXEC_FAILED, "exec failed");
  }
  std::string Message(int32_t statusCode) {
    auto iter = status_codes_.find(statusCode);
    if (iter == status_codes_.end()) {
      return "unknown";
    }
    return iter->second;
  }

 private:
  void Set(int32_t statusCode, std::string message) {
    auto iter = status_codes_.find(statusCode);
    assert(iter == status_codes_.end());
    status_codes_[statusCode] = message;
  }

 private:
  std::map<int32_t, std::string> status_codes_;
};
}  // namespace Common