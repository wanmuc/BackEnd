#pragma once

#include <string>

#include "log.hpp"
#include "robustio.hpp"

namespace Common {
class ServiceLock {
 public:
  static bool lock(std::string pidFile) {
    int fd;
    fd = open(pidFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0) {
      ERROR("open %s failed, error:%s", pidFile.c_str(), strerror(errno));
      return false;
    }
    int ret = lockf(fd, F_TEST, 0);  //返回0表示未加锁或者被当前进程加锁；返回-1表示被其他进程加锁
    if (ret < 0) {
      ERROR("lock %s failed, error:%s", pidFile.c_str(), strerror(errno));
      return false;
    }
    ret = lockf(fd, F_TLOCK, 0);  //尝试给文件加锁，加锁失败直接返回错误码，而不是一直阻塞
    if (ret < 0) {
      ERROR("lock %s failed, error:%s", pidFile.c_str(), strerror(errno));
      return false;
    }
    ftruncate(fd, 0);        //清空文件
    lseek(fd, 0, SEEK_SET);  //移动到文件开头
    char buf[1024] = {0};
    sprintf(buf, "%d", getpid());
    RobustIo robustIo(fd);
    robustIo.Write((uint8_t *)buf, strlen(buf));  // 写入进程的pid
    INFO("lock pidFile[%s] success. pid[%s]", pidFile.c_str(), buf);
    return true;
  }
};  // namespace ServiceLock
}  // namespace Common
