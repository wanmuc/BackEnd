#pragma once

#include "../../common/log.hpp"
#include "../../core/handler.hpp"
#include "../../core/mysvrclient.hpp"

class AccessHandler : public Core::MyHandler {
 public:
  void MySvrHandler(Protocol::MySvrMessage &req, Protocol::MySvrMessage &resp) {
    if (req.IsOneway()) {
      Core::MySvrClient().PushCallRaw(req);  // 直接推送请求到下游
    } else {
      Core::MySvrClient().RpcCallRaw(req, resp);  // 直接转发请求到下游
    }
  }

 private:
  bool isSupportRpc(std::string serviceName, std::string rpcName) override {
    if (serviceName == "Auth" or serviceName == "User") {
      return true;
    }
    return false;
  }
};