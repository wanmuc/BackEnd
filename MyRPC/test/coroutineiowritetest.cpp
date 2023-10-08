#include "../core/coroutine.h"
#include "../core/coroutineio.hpp"
#include "unittestcore.h"

class SocketIoMockWriteNormal : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) { return count; }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoWriteNormal(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoWrite(1, nullptr, 111);
  std::cout << "finish CoWriteNormal, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoWriteNormal) {
  SocketIoMockWriteNormal mockWriteNormal;
  SYSTEM.SetIoMock(&mockWriteNormal);
  ssize_t result;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoWriteNormal, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, 111);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockWriteError : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EBADF;
      return -1;
    }
    return count;
  }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoWriteError(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoWrite(1, nullptr, 111);
  assert(errno == EBADF);
  std::cout << "finish CoWriteError, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoWriteError) {
  SocketIoMockWriteError mockWriteError;
  SYSTEM.SetIoMock(&mockWriteError);
  ssize_t result;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoWriteError, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, -1);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockWriteEINTR : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EINTR;
      return -1;
    }
    return count;
  }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoWriteEINTR(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoWrite(1, nullptr, 444);
  std::cout << "finish CoWriteEINTR, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoWriteEINTR) {
  SocketIoMockWriteEINTR mockWriteEintr;
  SYSTEM.SetIoMock(&mockWriteEintr);
  ssize_t result;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoWriteEINTR, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, 444);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockWriteYield : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EWOULDBLOCK;
      return -1;
    }
    return count;
  }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoWriteYield(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoWrite(1, nullptr, 666);
  std::cout << "finish CoWriteYield, result = " << *result << std::endl;
}

TEST_CASE(Coroutineio_CoWriteYield) {
  SocketIoMockWriteYield mockWriteYield;
  SYSTEM.SetIoMock(&mockWriteYield);
  ssize_t result;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoWriteYield, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  MyCoroutine::CoroutineResume(SCHEDULE);
  ASSERT_EQ(result, 666);
  MyCoroutine::ScheduleClean(SCHEDULE);
}

class SocketIoMockWriteTimeOut : public Core::SocketIoMock {
 public:
  ssize_t read(int fd, void* buf, size_t count) { return count; }
  ssize_t write(int fd, const void* buf, size_t count) {
    static int temp = 0;
    temp++;
    if (1 == temp) {
      errno = EWOULDBLOCK;
      return -1;
    }
    return count;
  }
  int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) { return 0; }
};

void CoWriteTimeOut(void* arg) {
  ssize_t* result = (ssize_t*)arg;
  RpcTimeOut.Set(Core::TimeOut());
  EpollFd.Set(epoll_create(1));
  *result = Core::CoWrite(1, nullptr, 666);
  assert(*result == -1);
  assert(errno == EAGAIN);
  int temp = errno;
  std::cout << "finish CoWriteTimeOut, result = " << *result << std::endl;
  std::cout << "error msg = " << strerror(temp) << std::endl;
}

TEST_CASE(Coroutineio_CoWriteTimeOut) {
  Core::TimerData timerData;
  while (TIMER.GetLastTimer(timerData)) {
    TIMER.Run(timerData);
  }
  SocketIoMockWriteTimeOut mockWriteTimeOut;
  SYSTEM.SetIoMock(&mockWriteTimeOut);
  ssize_t result;
  MyCoroutine::ScheduleInit(SCHEDULE, 100, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, CoWriteTimeOut, &result);
  MyCoroutine::CoroutineResume(SCHEDULE);
  bool get = TIMER.GetLastTimer(timerData);
  ASSERT_TRUE(get);
  usleep(TIMER.TimeOutMs(timerData) * 1000);
  TIMER.Run(timerData);
  ASSERT_EQ(result, -1);
  MyCoroutine::ScheduleClean(SCHEDULE);
}
