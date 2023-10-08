#pragma once

#include "../common/config.hpp"
#include "eventdispatch.hpp"
#include "handler.hpp"

namespace Core {
class Reactor {
 public:
  void Run(Common::Config *config) {
    int64_t port;
    std::string listenIf;
    int64_t coroutineCount;
    config->GetIntValue("MyRPC", "port", port, 0);
    config->GetStrValue("MyRPC", "listen_if", listenIf, "eth0");
    config->GetIntValue("MyRPC", "coroutine_count", coroutineCount, 1024);
    event_dispatch_.Run(listenIf, port, coroutineCount);  // 陷入事件监听和分发的死循环
  }
  void RegHandler(MyHandler *handler) { event_dispatch_.RegHandler(handler); }

 private:
  EventDispatch event_dispatch_;  // 事件分发器
};
}  // namespace Core