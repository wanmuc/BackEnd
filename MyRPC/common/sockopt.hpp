#pragma once

#include <assert.h>
#include <sys/types.h>

namespace Common {
class SockOpt {
 public:
  static void GetBufSize(int sockFd, int &readBufSize, int &writeBufSize) {
    int value = 0;
    socklen_t optlen;
    optlen = sizeof(value);
    assert(0 == getsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, &value, &optlen));
    readBufSize = value;
    assert(0 == getsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, &value, &optlen));
    writeBufSize = value;
  }
  static void SetBufSize(int sockFd, int readBufSize, int writeBufSize) {
    assert(0 == setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, &readBufSize, sizeof(readBufSize)));
    assert(0 == setsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, &writeBufSize, sizeof(writeBufSize)));
  }
  static void EnableKeepAlive(int sockFd, int idleTime, int interval, int cnt) {
    int val = 1;
    socklen_t len = (socklen_t)sizeof(val);
    assert(0 == setsockopt(sockFd, SOL_SOCKET, SO_KEEPALIVE, (void *)&val, len));
    val = idleTime;
    assert(0 == setsockopt(sockFd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&val, len));
    val = interval;
    assert(0 == setsockopt(sockFd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&val, len));
    val = cnt;
    assert(0 == setsockopt(sockFd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&val, len));
  }
  static void DisableNagle(int sockFd) {
    int noDelay = 1;
    socklen_t len = (socklen_t)sizeof(noDelay);
    assert(0 == setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, &noDelay, len));
  }
  static int GetSocketError(int sockFd) {
    int err = 0;
    socklen_t errLen = sizeof(err);
    assert(0 == getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &err, &errLen));
    return err;
  }
};  // namespace SockOpt
}  // namespace Common
