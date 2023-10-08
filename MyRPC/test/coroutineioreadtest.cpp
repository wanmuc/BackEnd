#include "../core/coroutine.h"
#include "../core/coroutineio.hpp"
#include "unittestcore.h"

class SocketIoMockReadNormal : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoReadNormal(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoRead(1, nullptr, 10);
  std::cout << "finish CoReadNormal, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoReadNormal) {
  SocketIoMockReadNormal mockRead;
  SYSTEM.SetIoMock(&mockRead);
  ssize_t result;
  Core::TimerData timerData;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoReadNormal, &result);
  while (MyCoroutine::ScheduleRunning(SCHEDULE)) {
    bool get = TIMER.GetLastTimer(timerData);
    if (get) {
      usleep(TIMER.TimeOutMs(timerData) * 1000);
      TIMER.Run(timerData);
    }
    MyCoroutine::CoroutineResume(SCHEDULE);
  }
  ASSERT_EQ(result, 10);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockReadError : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) {
    errno = EBADF;
    return -1;
  }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoReadError(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoRead(1, nullptr, 10);
  std::cout << "finish CoReadError, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoReadError) {
  SocketIoMockReadError mockReadError;
  SYSTEM.SetIoMock(&mockReadError);
  ssize_t result;
  Core::TimerData timerData;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoReadError, &result);
  while (MyCoroutine::ScheduleRunning(SCHEDULE)) {
    bool get = TIMER.GetLastTimer(timerData);
    if (get) {
      usleep(TIMER.TimeOutMs(timerData) * 1000);
      TIMER.Run(timerData);
    }
    MyCoroutine::CoroutineResume(SCHEDULE);
  }
  ASSERT_EQ(result, -1);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockReadEINTR : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EINTR;
      return -1;
    }
    errno = 0;
    return count;
  }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoReadEINTR(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoRead(1, nullptr, 888);
  std::cout << "finish CoReadEINTR, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoReadEINTR) {
  SocketIoMockReadEINTR mockReadEINTR;
  SYSTEM.SetIoMock(&mockReadEINTR);
  ssize_t result;
  Core::TimerData timerData;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoReadEINTR, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, 888);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockReadYield : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EAGAIN;
      return -1;
    }
    errno = 0;
    return count;
  }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoReadYield(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoRead(1, nullptr, 666);
  std::cout << "finish CoReadYield, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoReadNormalYield) {
  SocketIoMockReadYield mockReadYield;
  SYSTEM.SetIoMock(&mockReadYield);
  ssize_t result;
  Core::TimerData timerData;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoReadYield, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  bool get = TIMER.GetLastTimer(timerData);
  ASSERT_TRUE(get);
  MyCoroutine::CoroutineResume(SCHEDULE);
  get = TIMER.GetLastTimer(timerData);
  ASSERT_FALSE(get);
  ASSERT_EQ(result, 666);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockReadTimeOut : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EAGAIN;
      return -1;
    }
    errno = 0;
    return count;
  }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoReadTimeOut(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoRead(1, nullptr, 168);
  assert(*result == -1);
  assert(errno == EAGAIN);
  int temp = errno;
  std::cout << "finish CoReadTimeOut, result = " << *result << std::endl;
  std::cout << "error msg = " << strerror(temp) << std::endl;
}

TEST_CASE(Coroutineio_CoReadTimeOut) {
  Core::TimerData timerData;
  // 清理掉存量的定时器
  while (TIMER.GetLastTimer(timerData)) {
    TIMER.Run(timerData);
  }
  SocketIoMockReadTimeOut mockReadTimeOut;
  SYSTEM.SetIoMock(&mockReadTimeOut);
  ssize_t result;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoReadTimeOut, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  bool get = TIMER.GetLastTimer(timerData);
  ASSERT_TRUE(get);
  usleep(TIMER.TimeOutMs(timerData) * 1000);
  TIMER.Run(timerData);
  MyCoroutine::ScheduleClean(SCHEDULE);
}
