#pragma once

#include <sys/epoll.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "../common/log.hpp"
#include "../common/utils.hpp"
#include "connmanager.hpp"
#include "coroutinelocal.hpp"
#include "epollctl.hpp"
#include "handler.hpp"
#include "timer.hpp"

extern Core::CoroutineLocal<int> EpollFd;

namespace Core {
class EventDispatch {
 public:
  void Run(std::string listenIf, int64_t port, int64_t coroutineCount) {
    std::thread(subHandler, coroutineCount, this).detach();  // 启动subReactor,这里需要调用detach，让创建的线程独立运行
    mainHandler(listenIf, port);                             // 启动mainReactor
  }
  void RegHandler(MyHandler *handler) { handler_ = handler; }

 private:
  void waitSubReactor() {
    std::unique_lock<std::mutex> locker(mutex_);
    cond_.wait(locker, [this]() -> bool { return sub_reactor_run_; });
  }
  void subReactorNotify() {
    std::unique_lock<std::mutex> locker(mutex_);
    sub_reactor_run_ = true;
    cond_.notify_one();
  }
  static void clearEventAndDelete(void *data) {
    EventData *eventData = (EventData *)data;
    EpollCtl::ClearEvent(eventData->epoll_fd_, eventData->fd_);  // 超时清除事件的监听，关闭连接
    delete eventData;                                            // 释放空间
  }
  void mainHandler(std::string listenIf, int64_t port) {
    waitSubReactor();  // 等待subReactor协程都启动完毕
    listen_sock_fd_ = createListenSocket(listenIf, (int)port);
    assert(listen_sock_fd_ > 0);
    epoll_event events[2048];
    main_epoll_fd_ = epoll_create(1);
    assert(main_epoll_fd_ > 0);
    EventData eventData(listen_sock_fd_, main_epoll_fd_, LISTEN);
    Common::Utils::SetNotBlock(listen_sock_fd_);
    EpollCtl::AddReadEvent(main_epoll_fd_, listen_sock_fd_, &eventData);
    int msec = -1;
    TimerData timerData;
    bool oneTimer = false;
    while (true) {
      oneTimer = idle_connection_timer_.GetLastTimer(timerData);
      if (oneTimer) {
        msec = idle_connection_timer_.TimeOutMs(timerData);
      }
      int num = epoll_wait(main_epoll_fd_, events, 2048, msec);
      if (num < 0) {
        ERROR("epoll_wait failed, errMsg[%s]", strerror(errno));
        continue;
      } else if (num == 0) {  // 没有事件了，下次调用epoll_wait大概率被挂起
        sleep(0);  // 这里直接sleep(0)让出cpu。大概率被挂起，这里主动让出cpu，可以减少一次epoll_wait的调用
        msec = -1;  // 大概率被挂起，故这里超时时间设置为-1
      } else {
        msec = 0;  // 下次大概率还有事件，故msec设置为0
      }
      for (int i = 0; i < num; i++) {
        EventData *data = (EventData *)events[i].data.ptr;
        data->events_ = events[i].events;
        mainEventHandler(data);
      }
      if (oneTimer) idle_connection_timer_.Run(timerData);  // 处理定时器
    }
  }
  static void subHandler(int coroutineCount, EventDispatch *eventDispatch) {
    epoll_event events[2048];
    eventDispatch->sub_epoll_fd_ = epoll_create(1);
    assert(eventDispatch->sub_epoll_fd_ > 0);
    eventDispatch->subReactorNotify();
    MyCoroutine::ScheduleInit(SCHEDULE, coroutineCount, 64 * 1024);
    int msec = -1;
    TimerData timerData;
    bool oneTimer = false;
    while (true) {
      oneTimer = TIMER.GetLastTimer(timerData);
      if (oneTimer) {
        msec = TIMER.TimeOutMs(timerData);
      }
      int num = epoll_wait(eventDispatch->sub_epoll_fd_, events, 2048, msec);
      if (num < 0) {
        ERROR("epoll_wait failed, errMsg[%s]", strerror(errno));
        continue;
      } else if (num == 0) {  // 没有事件了，下次调用epoll_wait大概率被挂起
        sleep(0);  // 这里直接sleep(0)让出cpu，大概率被挂起，这里主动让出cpu，可以减少一次epoll_wait的调用
        msec = -1;  // 大概率被挂起，故这里超时时间设置为-1
      } else {
        msec = 0;  // 下次大概率还有事件，故msec设置为0
      }
      for (int i = 0; i < num; i++) {
        EventData *eventData = (EventData *)events[i].data.ptr;
        eventData->events_ = events[i].events;
        eventDispatch->subEventHandler(eventData);
      }
      if (oneTimer) TIMER.Run(timerData);               // 处理定时器
      MyCoroutine::ScheduleTryReleaseMemory(SCHEDULE);  // 尝试释放协程池的内存
    }
  }
  static void coroutineEventEntry(void *arg) {
    EventData *eventData = (EventData *)arg;
    MyHandler *handler = (MyHandler *)eventData->handler_;
    EpollFd.Set(eventData->epoll_fd_);  // 把epoll实例fd，设置为协程本地变量
    handler->HandlerEntry(eventData);
  }
  void subEventHandler(EventData *eventData) {
    int cid = eventData->cid_;
    if (RPC_CLIENT == eventData->type_) {
      MyCoroutine::CoroutineResumeById(SCHEDULE, eventData->cid_);  // 唤醒之前主动让出cpu的协程
    } else if (CLIENT == eventData->type_) {
      if (eventData->cid_ == MyCoroutine::INVALID_ROUTINE_ID) {  // 没有运行的协程关联，则创建协程
        if (MyCoroutine::CoroutineCanCreate(SCHEDULE)) {
          eventData->cid_ = MyCoroutine::CoroutineCreate(SCHEDULE, coroutineEventEntry, eventData, 0);  // 创建协程
          cid = eventData->cid_;
        } else {  // 协程满之后，直接清除事件监听，关闭连接，释放空间
          WARN("MyCoroutine is full");
          clearEventAndDelete(eventData);
          return;
        }
      }
      MyCoroutine::CoroutineResumeById(SCHEDULE, cid);  // 唤醒协程
    }
    MyCoroutine::CoroutineResumeInBatch(SCHEDULE, cid);  // 如果有插入batch卡点，则唤醒batch卡点关联的协程
    MyCoroutine::CoroutineResumeBatchFinish(SCHEDULE);  // 尝试唤醒batch都已经执行完的协程。
  }
  void mainEventHandler(EventData *eventData) {
    if (LISTEN == eventData->type_) {
      return loopAccept(2048);  // 执行到这里就是有客户端的连接到来了，循环接受客户端的连接
    }
    // 客户端有可读事件，把客户端连接读写事件监听迁移到sub_epoll_fd_中，并取消超时定时器
    idle_connection_timer_.Cancel(eventData->timer_id_);
    EpollCtl::ClearEvent(main_epoll_fd_, eventData->fd_, false);
    eventData->handler_ = handler_;
    eventData->epoll_fd_ = sub_epoll_fd_;
    EpollCtl::AddReadEvent(sub_epoll_fd_, eventData->fd_, eventData);  // 监听可读事件，添加到sub_epoll_fd_中
  }
  int createListenSocket(std::string listenIf, int port) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = Common::Utils::GetAddr(listenIf);
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
      ERROR("socket failed. errMsg[%s]", strerror(errno));
      return -1;
    }
    int reuse = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) != 0) {
      ERROR("setsockopt failed. errMsg[%s]", strerror(errno));
      return -1;
    }
    if (bind(sockFd, (sockaddr *)&addr, sizeof(addr)) != 0) {
      ERROR("bind failed. errMsg[%s]", strerror(errno));
      return -1;
    }
    if (listen(sockFd, 2048) != 0) {
      ERROR("listen failed. errMsg[%s]", strerror(errno));
      return -1;
    }
    return sockFd;
  }
  void loopAccept(int maxConn) {  // 调用本函数之前需要把sockFd设置成非阻塞的
    while (maxConn--) {
      int clientFd = accept(listen_sock_fd_, NULL, 0);
      if (clientFd > 0) {
        Common::Utils::SetNotBlock(clientFd);
        Common::SockOpt::DisableNagle(clientFd);
        Common::SockOpt::EnableKeepAlive(clientFd, 300, 12, 5);
        EventData *eventData = new EventData(clientFd, main_epoll_fd_, CLIENT);
        // 注册定时器，30秒超时
        eventData->timer_id_ = idle_connection_timer_.Register(clearEventAndDelete, eventData, 30000);
        EpollCtl::AddReadEvent(main_epoll_fd_, clientFd, eventData);  // 监听可读事件，添加到main_epoll_fd_中
        continue;
      }
      if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
        ERROR("accept failed. errMsg[%s]", strerror(errno));
      }
      break;
    }
  }

 private:
  MyHandler *handler_;           // 业务处理的handler
  int sub_epoll_fd_;             // epoll实例的fd，用于监听客户端的读写
  int main_epoll_fd_;            // epoll实例的fd，用于监听客户端连接
  int listen_sock_fd_;           // 开启网络监听的fd
  Timer idle_connection_timer_;  // 空闲连接定时器

  std::mutex mutex_;
  std::condition_variable cond_;
  bool sub_reactor_run_{false};
};
}  // namespace Core