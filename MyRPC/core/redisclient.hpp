#pragma once

#include <string>

#include "../common/defer.hpp"
#include "../common/timedeal.hpp"
#include "../protocol/rediscodec.hpp"
#include "client.hpp"
#include "connmanager.hpp"
#include "distributedtrace.hpp"

namespace Core {
class RedisClient : public Client {
 public:
  RedisClient(std::string passwd) : passwd_(passwd) {}
  bool Set(std::string key, std::string value, int64_t expireTime, std::string &error) {
    beforeExec();
    Common::TimeStat time_stat;
    Common::Defer defer([&time_stat, this]() {
      DistributedTrace::AddTraceInfo("Redis", "Set", time_stat.GetSpendTimeUs(), status_code_, message_);
    });
    cmd_.makeSetCmd(key, value, expireTime);
    if (not execAndCheck(error)) {
      return false;
    }
    return redis_reply_.IsOk();
  }
  bool Get(std::string key, std::string &value, std::string &error) {
    beforeExec();
    Common::TimeStat time_stat;
    Common::Defer defer([&time_stat, this]() {
      DistributedTrace::AddTraceInfo("Redis", "Get", time_stat.GetSpendTimeUs(), status_code_, message_);
    });
    cmd_.makeGetCmd(key);
    if (not execAndCheck(error)) {
      return false;
    }
    value = redis_reply_.Value();  // 要注意nil和empty的case
    return true;
  }
  bool Del(std::string key, int64_t &delCount, std::string &error) {
    beforeExec();
    Common::TimeStat time_stat;
    Common::Defer defer([&time_stat, this]() {
      DistributedTrace::AddTraceInfo("Redis", "Del", time_stat.GetSpendTimeUs(), status_code_, message_);
    });
    cmd_.makeDelCmd(key);
    if (not execAndCheck(error)) {
      return false;
    }
    delCount = atol(redis_reply_.Value().c_str());
    return true;
  }
  bool Incr(std::string key, int64_t &value, std::string &error) {
    beforeExec();
    Common::TimeStat time_stat;
    Common::Defer defer([&time_stat, this]() {
      DistributedTrace::AddTraceInfo("Redis", "Incr", time_stat.GetSpendTimeUs(), status_code_, message_);
    });
    cmd_.makeIncrCmd(key);
    if (not execAndCheck(error)) {
      return false;
    }
    value = atol(redis_reply_.Value().c_str());
    return true;
  }

 private:
  bool execAndCheck(std::string &error) {
    if (not execRedisCommand(error)) {
      return false;
    }
    if (redis_reply_.IsError()) {
      error = redis_reply_.Value();
      message_ = error;
      status_code_ = EXEC_FAILED;
      return false;
    }
    return true;
  }
  bool execAuth(Conn *conn, std::string &error) {
    Protocol::RedisCodec codec;
    Protocol::RedisCommand cmd;
    cmd.makeAuthCmd(passwd_);
    Protocol::RedisReply *reply = nullptr;
    auto errorDeal = [&error, this](int status_code, std::string desc) {
      error = desc;
      message_ = desc;
      status_code_ = status_code;
    };
    if (not Call(conn, codec, &cmd, (void **)&reply, errorDeal)) {
      return false;
    }
    if (reply->IsError() || (not reply->IsOk())) {
      error = reply->Value();
      message_ = error;
      status_code_ = EXEC_FAILED;
      delete reply;
      return false;
    }
    delete reply;
    conn->finish_auth_ = true;
    return true;
  }
  bool execRedisCommand(std::string &error) {
    std::string serviceName = "redis";
    auto errorDeal = [&error, this](int status_code, std::string desc) {
      error = desc;
      message_ = desc;
      status_code_ = status_code;
    };
    auto execAuthCallBack = [this](Conn *conn, std::string &callBackError) -> bool {
      if (conn->finish_auth_) {
        return true;
      }
      return execAuth(conn, callBackError);
    };
    Protocol::RedisCodec codec;
    Protocol::RedisReply *reply = nullptr;
    if (not CallRetry(serviceName, codec, &cmd_, (void **)&reply, execAuthCallBack, errorDeal)) {
      return false;
    }
    redis_reply_ = *reply;
    delete reply;
    return true;
  }
  void beforeExec() {
    status_code_ = 0;
    message_ = "success";
  }

 private:
  std::string passwd_;
  int32_t status_code_{0};
  std::string message_{"success"};
  Protocol::RedisCommand cmd_;
  Protocol::RedisReply redis_reply_;
};
}  // namespace Core