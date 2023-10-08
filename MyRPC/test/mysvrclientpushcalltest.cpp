#include "../core/epollctl.hpp"
#include "../core/mysvrclient.hpp"
#include "../core/timer.hpp"
#include "../service/echo/proto/echo.pb.h"
#include "unittestcore.h"

static int epollFd = 0;

static void PushCallSuccess(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  MySvr::Echo::OneWayMessage oneWayMessage;
  int ret = Core::MySvrClient().PushCall(oneWayMessage);
  assert(0 == ret);
  *(bool*)data = false;  // 设置成退出循环
}

static void eventHandler(Core::EventData* eventData) {
  int cid = eventData->cid_;
  assert(cid != MyCoroutine::INVALID_ROUTINE_ID);
  MyCoroutine::CoroutineResumeById(SCHEDULE, cid);  // 唤醒之前主动让出cpu的协程
}

TEST_CASE(MySvrClient_PushCallSuccess) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SYSTEM.SetIoMock(nullptr);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, PushCallSuccess, &run);
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

class PushCallConnectFailed : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    errno = EBADF;
    return -1;
  }
};

static void PushCallFailedConnect(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  MySvr::Echo::OneWayMessage oneWayMessage;
  CONN_MANAGER.SetMaxIdleTime(1);
  sleep(2);  // 让之前的连接都超时
  int ret = Core::MySvrClient().PushCall(oneWayMessage);
  assert(CONNECTION_FAILED == ret);
  *(bool*)data = false;  // 设置成退出循环
}

TEST_CASE(MySvrClient_PushCallFailedConnect) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  PushCallConnectFailed connectFailed;
  SYSTEM.SetIoMock(&connectFailed);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, PushCallFailedConnect, &run);
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

class PushCallWriteFailed : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) {
    errno = EBADF;
    return -1;
  }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

static void PushCallFailedWrite(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  MySvr::Echo::OneWayMessage oneWayMessage;
  CONN_MANAGER.SetMaxIdleTime(1);
  sleep(2);  // 让之前的连接都超时
  int ret = Core::MySvrClient().PushCall(oneWayMessage);
  assert(WRITE_FAILED == ret);
  *(bool*)data = false;  // 设置成退出循环
}

TEST_CASE(MySvrClient_PushCallFailedWrite) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  PushCallWriteFailed writeFailed;
  SYSTEM.SetIoMock(&writeFailed);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, PushCallFailedWrite, &run);
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