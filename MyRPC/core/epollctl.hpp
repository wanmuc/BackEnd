#pragma once

#include <assert.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <map>
#include <string>

namespace Core {
// 监听的逻辑类型
enum EventType {
  LISTEN = 1,      // listen fd的事件监听
  CLIENT = 2,      // 客户端事件的监听
  RPC_CLIENT = 3,  // rpc客户端读写的监听
};

struct EventData {
  EventData(int fd, int epoll_fd, int type) : fd_(fd), epoll_fd_(epoll_fd), type_(type) {}
  int fd_{0};
  int epoll_fd_{0};
  int type_;                // 监听的逻辑类型
  uint32_t events_{0};      // epoll触发的具体事件
  int cid_{-1};             // 关联的协程id
  int64_t timer_id_{-1};    // mainReactor中用于关联空闲连接超时定时器的id
  void *handler_{nullptr};  // 客户端初始事件的处理入口
};

class EpollCtl {
 public:
  static void AddReadEvent(int epollFd, int fd, void *userData) {
    opEvent(epollFd, fd, userData, EPOLL_CTL_ADD, EPOLLIN);
  }
  static void AddWriteEvent(int epollFd, int fd, void *userData) {
    opEvent(epollFd, fd, userData, EPOLL_CTL_ADD, EPOLLOUT);
  }
  static void ModToReadEvent(int epollFd, int fd, void *userData) {
    opEvent(epollFd, fd, userData, EPOLL_CTL_MOD, EPOLLIN);
  }
  static void ModToWriteEvent(int epollFd, int fd, void *userData) {
    opEvent(epollFd, fd, userData, EPOLL_CTL_MOD, EPOLLOUT);
  }
  static void ClearEvent(int epollFd, int fd, bool isClose = true) {
    assert(epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) != -1);
    if (isClose) close(fd);  // close操作需要EPOLL_CTL_DEL之后调用，否则调用epoll_ctl()删除fd会失败
  }
  static std::string EventReadable(int events) {
    static std::map<uint32_t, std::string> event2Str = {
        {EPOLLIN, "EPOLLIN"},   {EPOLLOUT, "EPOLLOUT"}, {EPOLLRDHUP, "EPOLLRDHUP"}, {EPOLLPRI, "EPOLLPRI"},
        {EPOLLERR, "EPOLLERR"}, {EPOLLHUP, "EPOLLHUP"}, {EPOLLET, "EPOLLET"},       {EPOLLONESHOT, "EPOLLONESHOT"}};
    std::string result = "";
    auto addEvent = [&result](std::string event) {
      if ("" == result) {
        result = event;
      } else {
        result += "|";
        result += event;
      }
    };
    for (auto &item : event2Str) {
      if (events & item.first) {
        addEvent(item.second);
      }
    }
    return result;
  }

 private:
  static void opEvent(int epollFd, int fd, void *userData, int op, uint32_t events) {
    epoll_event event;
    event.data.ptr = userData;
    event.events = events;
    assert(epoll_ctl(epollFd, op, fd, &event) != -1);
  }
};
}  // namespace Core