#include "../core/epollctl.hpp"
#include "../core/mysvrclient.hpp"
#include "../core/timer.hpp"
#include "../service/echo/proto/echo.pb.h"
#include "unittestcore.h"

static int epollFd = 0;

static void RpcCallSuccess(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  MySvr::Echo::EchoMySelfRequest request;
  MySvr::Echo::EchoMySelfResponse response;
  int ret = Core::MySvrClient().RpcCall(request, response);
  assert(0 == ret);
  ret = Core::MySvrClient().RpcCall(request, response, true);
  assert(0 == ret);
  *(bool*)data = false;  // 设置成退出循环
}

static void RpcCallConnectionTimeOut(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  MySvr::Echo::EchoMySelfRequest request;
  MySvr::Echo::EchoMySelfResponse response;
  int ret = Core::MySvrClient().RpcCall(request, response);
  assert(CONNECTION_FAILED == ret);
  *(bool*)data = false;  // 设置成退出循环
  std::cout << "RpcCallConnectionTimeOut set result false" << std::endl;
}

static void RpcCallFailedWrite(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  MySvr::Echo::EchoMySelfRequest request;
  MySvr::Echo::EchoMySelfResponse response;
  int ret = Core::MySvrClient().RpcCall(request, response);
  assert(WRITE_FAILED == ret);
  *(bool*)data = false;  // 设置成退出循环
}

static void RpcCallFailedRead(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  MySvr::Echo::EchoMySelfRequest request;
  MySvr::Echo::EchoMySelfResponse response;
  int ret = Core::MySvrClient().RpcCall(request, response);
  assert(READ_FAILED == ret);
  *(bool*)data = false;  // 设置成退出循环
}

class RpcCallConnectTimeOut : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    sleep(1);
    errno = EINPROGRESS;
    return -1;
  }
};

class RpcCallReadFailed : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) {
    errno = EBADF;
    return -1;
  }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

class RpcCallWriteFailed : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) {
    errno = EBADF;
    return -1;
  }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

static void eventHandler(Core::EventData* eventData) {
  int cid = eventData->cid_;
  assert(cid != MyCoroutine::INVALID_ROUTINE_ID);
  MyCoroutine::CoroutineResumeById(SCHEDULE, cid);  // 唤醒之前主动让出cpu的协程
}

TEST_CASE(MySvrClient_RpcCallWriteFailed) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  RpcCallWriteFailed writeFailed;
  SYSTEM.SetIoMock(&writeFailed);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, RpcCallFailedWrite, &run);
  MyCoroutine::CoroutineResume(SCHEDULE);
  epoll_event events[2048];
  int msec = -1;
  while (run) {
    bool oneTimer = TIMER.GetLastTimer(timerData);
    if (oneTimer) {
      msec = TIMER.TimeOutMs(timerData);
    }
    int num = epoll_wait(epollFd, events, 2048, msec);
    if (num < 0) {
      continue;
    }
    for (int i = 0; i < num; i++) {
      eventHandler((Core::EventData*)events[i].data.ptr);
    }
    if (oneTimer) {
      TIMER.Run(timerData);
    }
  }
  ASSERT_FALSE(run);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

TEST_CASE(MySvrClient_RpcCallReadFailed) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  RpcCallReadFailed readFailed;
  SYSTEM.SetIoMock(&readFailed);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, RpcCallFailedRead, &run);
  MyCoroutine::CoroutineResume(SCHEDULE);
  epoll_event events[2048];
  int msec = -1;
  while (run) {
    bool oneTimer = TIMER.GetLastTimer(timerData);
    if (oneTimer) {
      msec = TIMER.TimeOutMs(timerData);
    }
    int num = epoll_wait(epollFd, events, 2048, msec);
    if (num < 0) {
      continue;
    }
    for (int i = 0; i < num; i++) {
      eventHandler((Core::EventData*)events[i].data.ptr);
    }
    if (oneTimer) {
      TIMER.Run(timerData);
    }
  }
  ASSERT_FALSE(run);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

TEST_CASE(MySvrClient_RpcCallConnectionTimeOut) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  RpcCallConnectTimeOut connectTimeOut;
  SYSTEM.SetIoMock(&connectTimeOut);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, RpcCallConnectionTimeOut, &run);
  MyCoroutine::CoroutineResume(SCHEDULE);
  // rpc调用和connect都做了3次重试，故这里要构造9次connect超时
  for (int i = 1; i <= 9; i++) {
    bool oneTimer = TIMER.GetLastTimer(timerData);
    ASSERT_TRUE(oneTimer);
    std::cout << "time out ms = " << TIMER.TimeOutMs(timerData) << std::endl;
    TIMER.Run(timerData);
  }
  ASSERT_FALSE(run);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

TEST_CASE(MySvrClient_RpcCallSuccess) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SYSTEM.SetIoMock(nullptr);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, RpcCallSuccess, &run);
  MyCoroutine::CoroutineResume(SCHEDULE);
  epoll_event events[2048];
  int msec = -1;
  while (run) {
    bool oneTimer = TIMER.GetLastTimer(timerData);
    if (oneTimer) {
      msec = TIMER.TimeOutMs(timerData);
    }
    int num = epoll_wait(epollFd, events, 2048, msec);
    if (num < 0) {
      continue;
    }
    for (int i = 0; i < num; i++) {
      eventHandler((Core::EventData*)events[i].data.ptr);
    }
    if (oneTimer) {
      TIMER.Run(timerData);
    }
  }
  ASSERT_FALSE(run);
  MyCoroutine::ScheduleClean(SCHEDULE);
}
