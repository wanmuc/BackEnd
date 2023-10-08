#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "codec.hpp"

namespace Protocol {
// 只支持4种应答类型
enum RedisReplyType {
  SIMPLE_STRINGS = 1,  // 简单字符串，响应的首字节是 "+"，demo: "+OK\r\n"
  ERRORS = 2,          // 错误，响应的首字节是 "-"，demo："-Error message\r\n"
  INTEGERS = 3,        // 整型， 响应的首字节是 ":"，demo：":1000\r\n"
  BULK_STRINGS = 4,    // 批量字符串，相应的首字节是 "$"，demo："$2\r\nok\r\n"，"$0\r\n\r\n"，"$-1\r\n"
};
/* 只支持5种命令
  SET  // 设置key，value
  GET  // 获取key，对应的value值
  DEL  // 删除key
  INCR // 数值递增1
  AUTH // 认证
 */
typedef struct RedisCommand {
  void makeGetCmd(std::string key) {
    params_.push_back("GET");
    params_.push_back(key);
  }
  void makeDelCmd(std::string key) {
    params_.push_back("DEL");
    params_.push_back(key);
  }
  void makeAuthCmd(std::string passwd) {
    params_.push_back("AUTH");
    params_.push_back(passwd);
  }
  void makeSetCmd(std::string key, std::string value, int64_t expireTime = 0) {
    params_.push_back("SET");
    params_.push_back(key);
    params_.push_back(value);
    if (expireTime > 0) {
      params_.push_back("EX");
      params_.push_back(std::to_string(expireTime));
    }
  }
  void makeIncrCmd(std::string key) {
    params_.push_back("INCR");
    params_.push_back(key);
  }
  void GetOut(std::string &str) {
    size_t len = params_.size();
    std::stringstream out;
    out << "*" << len << "\r\n";
    for (size_t i = 0; i < params_.size(); i++) {
      out << "$" << params_[i].size() << "\r\n";
      out << params_[i] << "\r\n";
    }
    str = out.str();
  }
  std::vector<std::string> params_;
} RedisCommand;

typedef struct RedisReply {
  bool IsOk() { return value_ == "OK"; }
  bool IsError() { return type_ == ERRORS; }
  bool IsNull() { return is_null_; }
  std::string Value() { return value_; }
  int64_t IntValue() {
    assert(type_ == INTEGERS);
    return std::stol(value_);
  }

  RedisReplyType type_;
  std::string value_;
  bool is_null_{false};
} RedisReply;
}  // namespace Protocol
