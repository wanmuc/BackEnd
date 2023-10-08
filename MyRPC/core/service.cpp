#include "service.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../common/cmdline.h"
#include "../common/log.hpp"
#include "../common/servicelock.hpp"
#include "../common/utils.hpp"
#include "coroutinelocal.hpp"
#include "signalhandler.hpp"

Core::CoroutineLocal<int> EpollFd;                  // subReactor关联的epoll实例fd
Core::CoroutineLocal<Core::TimeOut> RpcTimeOut;     // rpc调用超时配置
Core::CoroutineLocal<MySvr::Base::Context> ReqCtx;  // 当前请求关联的上下文

namespace Core {
int MyRPCService::Init(int argc, char* argv[]) {
  if (parseInitArgs(argc, argv) != 0) return -1;  // 解析命令行参数
  if (is_daemon_) assert(0 == daemon(0, 0));
  Core::Signal::SignalDealReg();    // 注册信号处理函数
  Common::Utils::CoreDumpEnable();  // 开启core dump
  std::string programName = Common::Utils::GetSelfName();
  const char* str = programName.c_str();
  std::string cfgFile = Common::Strings::StrFormat((char*)"/home/backend/service/%s/%s.conf", str, str);
  assert(config_.Load(cfgFile));  // 加载配置文件
  config_.Dump([](const std::string& section, const std::string& key, const std::string& value) {
    INFO("section[%s],keyValue[%s=%s]", section.c_str(), key.c_str(), value.c_str());
  });
  return 0;
}

void MyRPCService::Run() {
  if (not Common::ServiceLock::lock("/home/backend/lock/subsys/" + Common::Utils::GetSelfName())) {
    ERROR("service already running");
    return;
  }
  if (is_debug_) {
    reactor_.Run(&config_);  // debug模式下，直接启动reactor，陷入事件监听
    return;
  }
  int64_t processCount = 0;
  config_.GetIntValue("MyRPC", "process_count", processCount, Common::Utils::GetNProcs());
  for (int64_t i = 0; i < processCount; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      ERROR("call fork failed. errMsg[%s]", strerror(errno));
      continue;
    }
    if (0 == pid) {  // 子进程直接跳出循环
      is_master_ = false;
      break;
    }
    pids_.push_back(pid);
  }
  if (is_master_) {
    monitorWorker();  // 主进程监控子进程
  } else {
    reactor_.Run(&config_);  // 子进程启动reactor，陷入事件监听
  }
}

void MyRPCService::Stop() {
  is_running_ = false;
  for (size_t i = 0; i < pids_.size(); i++) {
    waitpid(pids_[i], NULL, 0);
  }
  INFO("service finish stop, worker count[%d]", pids_.size());
}

void MyRPCService::usage() {
  std::cout << "Usage: " << Common::Utils::GetSelfName() << " [-d -debug]" << std::endl;
  std::cout << std::endl;
  std::cout << "   -h,--help       print usage" << std::endl;
  std::cout << "   -d              run in daemon mode" << std::endl;
  std::cout << "   -debug          run in debug mode" << std::endl;
  std::cout << std::endl;
}

void MyRPCService::monitorWorker() {
  while (true) {
    sleep(1);  // 每1秒check一下子进程的状态
    for (size_t i = 0; i < pids_.size(); i++) {
      if (pids_[i] <= 0 || kill(pids_[i], 0) != 0) {  // 子进程状态异常，则重启子进程
        pids_[i] = restartWorker(pids_[i]);
      }
    }
  }
}

pid_t MyRPCService::restartWorker(pid_t oldPid) {
  pid_t pid = fork();
  if (pid < 0) {
    ERROR("call fork failed. errMsg[%s]", strerror(errno));
    return -1;
  }
  if (0 == pid) {
    is_master_ = false;
    reactor_.Run(&config_);  // 子进程启动reactor，陷入事件监听，不会再返回
  }
  // 只有父进程会执行到这里
  INFO("worker process pid[%d] not exist, restart new pid[%d]", oldPid, pid);
  return pid;
}

int MyRPCService::parseInitArgs(int argc, char** argv) {
  Common::CmdLine::SetUsage(MyRPCService::usage);
  Common::CmdLine::BoolOpt(&is_daemon_, "d");
  Common::CmdLine::BoolOpt(&is_debug_, "debug");
  Common::CmdLine::Parse(argc, argv);
  return 0;
}
}  // namespace Core
