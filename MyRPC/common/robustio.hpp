#pragma once

#include <unistd.h>

#include "utils.hpp"

namespace Common {
class RobustIo {
 public:
  RobustIo(int fd) : fd_(fd) {}
  ssize_t Write(uint8_t* data, size_t len) {
    ssize_t total = len;
    while (total > 0) {
      ssize_t ret = write(fd_, data, total);
      if (ret <= 0) {
        if (0 == ret || RestartAgain(errno)) continue;
        return -1;
      }
      total -= ret;
      data += ret;
    }
    return len;
  }
  ssize_t Read(uint8_t* data, size_t len) {
    ssize_t total = len;
    while (total > 0) {
      ssize_t ret = read(fd_, data, total);
      if (0 == ret) break;
      if (ret < 0) {
        if (RestartAgain(errno)) continue;
        return -1;
      }
      total -= ret;
      data += ret;
    }
    return len - total;
  }
  void SetNotBlock() {
    Utils::SetNotBlock(fd_);
    is_block_ = false;
  }
  void SetTimeOut(int64_t timeOutSec, int64_t timeOutUSec) {
    if (not is_block_) return;  //非阻塞的不用设置sock的读写超时时间，设置了也无效果
    struct timeval tv {
      .tv_sec = timeOutSec, .tv_usec = timeOutUSec,
    };
    assert(setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != -1);
    assert(setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != -1);
  }
  bool RestartAgain(int err) {
    if (EINTR == err) return true;  // 被信号中断都可以重启读写
    if (is_block_) return false;    // 阻塞io的情况下，其他情况都不可以重启读写
    if (EAGAIN == err || EWOULDBLOCK == err) return true;  //非阻塞io的情况下，暂时io不可用才可以重新启动读写
    return false;
  }

 private:
  int fd_{-1};
  bool is_block_{true};  // fd_默认是阻塞的
};
}  // namespace Common