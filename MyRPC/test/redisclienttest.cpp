#include "../core/epollctl.hpp"
#include "../core/redisclient.hpp"
#include "../core/timer.hpp"
#include "unittestcore.h"

static int epollFd = 0;

static void SetAndGetSuccess(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  std::string error;
  bool result = Core::RedisClient("backend").Set("UnitTest_set_key", "UnitTest_set_value", 0, error);
  std::cout << "set error = " << error << std::endl;
  assert(result == true);
  assert(error == "");
  std::string value;
  result = Core::RedisClient("backend").Get("UnitTest_set_key", value, error);
  std::cout << "get error = " << error << std::endl;
  assert(result == true);
  assert(error == "");
  assert(value == "UnitTest_set_value");
  *(bool*)data = false;  // 设置成退出循环
}

static void DelSuccess(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  std::string error;
  bool result = Core::RedisClient("backend").Set("UnitTest_del_key", "UnitTest_del_value", 0, error);
  std::cout << "set error = " << error << std::endl;
  assert(result == true);
  assert(error == "");
  int64_t delCount = 0;
  result = Core::RedisClient("backend").Del("UnitTest_del_key", delCount, error);
  std::cout << "del error = " << error << std::endl;
  assert(result == true);
  assert(error == "");
  assert(delCount == 1);
  *(bool*)data = false;  // 设置成退出循环
}

static void IncrSuccess(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  std::string error;
  int64_t value = 0;
  bool result = Core::RedisClient("backend").Incr("UnitTest_incr_key", value, error);
  std::cout << "incr error = " << error << std::endl;
  std::cout << "incr value = " << value << std::endl;
  assert(result == true);
  assert(error == "");
  assert(value == 1);
  result = Core::RedisClient("backend").Del("UnitTest_incr_key", value, error);
  assert(result == true);
  *(bool*)data = false;  // 设置成退出循环
}

static void SetAuthFailed(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  CONN_MANAGER.SetMaxIdleTime(1);
  sleep(2);  // redis的连接过期，保证后面的连接是重新创建的，这样才能认证失败
  std::string error;
  bool result = Core::RedisClient("backend2").Set("UnitTest_set_key", "UnitTest_set_value", 0, error);
  std::cout << "set error = " << error << std::endl;
  assert(result == false);
  assert(error != "");
  *(bool*)data = false;  // 设置成退出循环
}

static void SetConnectionTimeOut(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  CONN_MANAGER.SetMaxIdleTime(1);
  sleep(2);  // redis的连接过期，保证后面的连接是重新创建的，这样才能认证失败
  std::string error;
  bool result = Core::RedisClient("backend").Set("UnitTest_set_key", "UnitTest_set_value", 0, error);
  std::cout << "set error = " << error << std::endl;
  assert(result == false);
  assert(error != "");
  assert(error == "get conn failed");
  *(bool*)data = false;  // 设置成退出循环
  std::cout << "SetConnectionTimeOut set result false" << std::endl;
}

static void SetReadFailed(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  CONN_MANAGER.SetMaxIdleTime(1);
  sleep(2);  // redis的连接过期，保证后面的连接是重新创建的，这样才能认证失败
  std::string error;
  bool result = Core::RedisClient("backend").Set("UnitTest_set_key", "UnitTest_set_value", 0, error);
  std::cout << "set error = " << error << std::endl;
  assert(result == false);
  assert(error != "");
  assert(error == "conn call back failed. CoRead failed. Bad file descriptor");
  *(bool*)data = false;  // 设置成退出循环
}

static void SetWriteFailed(void* data) {
  EpollFd.Set(epollFd);
  ReqCtx.Set(MySvr::Base::Context());
  CONN_MANAGER.SetMaxIdleTime(1);
  sleep(2);  // redis的连接过期，保证后面的连接是重新创建的，这样才能认证失败
  std::string error;
  bool result = Core::RedisClient("backend").Set("UnitTest_set_key", "UnitTest_set_value", 0, error);
  std::cout << "set error = " << error << std::endl;
  assert(result == false);
  assert(error != "");
  assert(error == "conn call back failed. CoWrite failed.Bad file descriptor");
  *(bool*)data = false;  // 设置成退出循环
}

static void eventHandler(Core::EventData* eventData) {
  int cid = eventData->cid_;
  assert(cid != MyCoroutine::INVALID_ROUTINE_ID);
  MyCoroutine::CoroutineResumeById(SCHEDULE, cid);  // 唤醒之前主动让出cpu的协程
}

class SetConnectTimeOut : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    sleep(1);
    errno = EINPROGRESS;
    return -1;
  }
};

class ReadFailed : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) {
    errno = EBADF;
    return -1;
  }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

class WriteFailed : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) {
    errno = EBADF;
    return -1;
  }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

TEST_CASE(RedisClient_IncrSuccess) {
  Core::TimerData timerData;
  // redis服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SYSTEM.SetIoMock(nullptr);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, IncrSuccess, &run);
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

TEST_CASE(RedisClient_DelSuccess) {
  Core::TimerData timerData;
  // redis服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SYSTEM.SetIoMock(nullptr);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, DelSuccess, &run);
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

TEST_CASE(RedisClient_SetAndGetSuccess) {
  Core::TimerData timerData;
  // redis服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SYSTEM.SetIoMock(nullptr);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, SetAndGetSuccess, &run);
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

TEST_CASE(RedisClient_SetAuthFailed) {
  Core::TimerData timerData;
  // redis服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SYSTEM.SetIoMock(nullptr);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, SetAuthFailed, &run);
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

TEST_CASE(RedisClient_SetConnectionTimeOut) {
  Core::TimerData timerData;
  // redis服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SetConnectTimeOut connectTimeOut;
  SYSTEM.SetIoMock(&connectTimeOut);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, SetConnectionTimeOut, &run);
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

TEST_CASE(RedisClient_SetReadFailed) {
  Core::TimerData timerData;
  // redis服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  ReadFailed readFailed;
  SYSTEM.SetIoMock(&readFailed);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, SetReadFailed, &run);
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

TEST_CASE(RedisClient_SetWriteFailed) {
  Core::TimerData timerData;
  // redis服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  WriteFailed writeFailed;
  SYSTEM.SetIoMock(&writeFailed);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, SetWriteFailed, &run);
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
