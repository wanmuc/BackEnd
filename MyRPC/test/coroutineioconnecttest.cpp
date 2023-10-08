#include "../core/coroutine.h"
#include "../core/coroutineio.hpp"
#include "unittestcore.h"

class SocketIoMockConnectNormal : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoConnectNormal(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoConnect(1, nullptr, 0);
  assert(*result == 0);
  std::cout << "finish CoConnectNormal, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoConnectNormal) {
  SocketIoMockConnectNormal mockConnectNormal;
  SYSTEM.SetIoMock(&mockConnectNormal);
  ssize_t result = -1;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoConnectNormal, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, 0);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockConnectEINTR : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EINTR;
      return -1;
    }
    return 0;
  }
};

void CoConnectEINTR(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoConnect(1, nullptr, 0);
  assert(*result == 0);
  std::cout << "finish CoConnectEINTR, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoConnectEINTR) {
  SocketIoMockConnectEINTR mockConnectEINTR;
  SYSTEM.SetIoMock(&mockConnectEINTR);
  ssize_t result = -1;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoConnectEINTR, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, 0);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockConnectError : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EBADF;
      return -1;
    }
    return 0;
  }
};

void CoConnectError(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoConnect(1, nullptr, 0);
  assert(*result == -1);
  assert(errno == EBADF);
  std::cout << "finish CoConnectError, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoConnectError) {
  SocketIoMockConnectError mockConnectError;
  SYSTEM.SetIoMock(&mockConnectError);
  ssize_t result = -1;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoConnectError, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, -1);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockConnectTimeOut : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EINPROGRESS;
      return -1;
    }
    return 0;
  }
};

void CoConnectTimeOut(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoConnect(1, nullptr, 0);
  assert(*result == -1);
  assert(errno == EAGAIN);
  int temp = errno;
  std::cout << "finish CoConnectTimeOut, result = " << *result << std::endl;
  std::cout << "error message = " << strerror(temp) << std::endl;
}

TEST_CASE(Coroutineio_CoConnectTimeOut) {
  Core::TimerData timerData;
  while (TIMER.GetLastTimer(timerData)) {
    TIMER.Run(timerData);
  }
  SocketIoMockConnectTimeOut mockConnectTimeOut;
  SYSTEM.SetIoMock(&mockConnectTimeOut);
  ssize_t result = 0;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoConnectTimeOut, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  bool get = TIMER.GetLastTimer(timerData);
  ASSERT_TRUE(get);
  std::cout << "time out ms = " << TIMER.TimeOutMs(timerData) << std::endl;
  usleep(TIMER.TimeOutMs(timerData) * 1000);
  TIMER.Run(timerData);
  ASSERT_EQ(result, -1);
  MyCoroutine::ScheduleClean(SCHEDULE);
}
