#pragma once

#include "../common/config.hpp"
#include "../common/singleton.hpp"
#include "handler.hpp"
#include "reactor.hpp"

#define SERVICE Common::Singleton<Core::MyRPCService>::Instance()

namespace Core {
class MyRPCService {
 public:
  int Init(int argc, char* argv[]);  // 初始化，从配置文件中读取框架相关的配置
  void Run();                        // 启动运行
  void Stop();                       // 停止运行

  bool IsRun() { return is_running_; }
  bool IsMaster() { return is_master_; }
  void RegHandler(MyHandler* handler) { reactor_.RegHandler(handler); }

 private:
  static void usage();
  void monitorWorker();
  pid_t restartWorker(pid_t oldPid);
  int parseInitArgs(int argc, char* argv[]);

 private:
  bool is_master_{true};     // 是否主进程
  bool is_running_{true};    // 是否运行中
  bool is_daemon_{false};    // 服务是否以守护进程的方式运行
  bool is_debug_{false};     // 服务是否进入调试模式
  Reactor reactor_;          // reactor类
  Common::Config config_;    // 配置文件
  std::vector<pid_t> pids_;  // 子进程pid
};
}  // namespace Core