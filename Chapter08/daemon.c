#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int daemonInit() {
  pid_t pid = 0;
  // 设置创建文件掩码为0
  umask(0);
  // 第一次调用fork创建子进程
  pid = fork();
  if (pid < 0) {
    printf("first call fork failed. errorMsg[%s]\n", strerror(errno));
    return -1;
  } else if (pid != 0) {
    exit(0);  // 第一次调用fork从父进程中返回，父进程直接退出
  }
  // 第一次调用fork从子进程返回，调用setsid创建新会话并脱离终端
  pid = setsid();
  if (pid < 0) {
    printf("call setsid failed. errorMsg[%s]\n", strerror(errno));
    return -1;
  }
  // 第二次调用fork创建孙子进程
  pid = fork();
  if (pid < 0) {
    printf("second call fork failed. errorMsg[%s]\n", strerror(errno));
    return -1;
  } else if (pid != 0) {
    exit(0);  // 第二次调用fork从子进程中返回，子进程直接退出
  }
  // 从孙子进程中返回，切换工作目录到"/"
  if (chdir("/") < 0) {
    printf("call chdir failed. errorMsg[%s]\n", strerror(errno));
    return -1;
  }
  int i = 0;
  // 关闭从子进程继承的文件描述符
  for (i = 0; i < getdtablesize(); i++) {
    close(i);
  }
  // 关闭所有的fd后，再连续打开3个fd
  // 因为open每次都返回进程最小的未打开的文件描述符
  // 相当于对标准输入(fd为0)，标准输出(fd为1)，标准错误(fd为2)
  // 重定向到"/dev/null"
  int fd0 = open("/dev/null", O_RDWR);
  int fd1 = open("/dev/null", O_RDWR);
  int fd2 = open("/dev/null", O_RDWR);
  if (!(0 == fd0 && 1 == fd1 && 2 == fd2)) {
    printf("unexpectd file desc %d %d %d\n", fd0, fd1, fd2);
    return -1;
  }
  return 0;
}

int main() {
  if (0 == daemonInit()) {
    while (1) {
      sleep(1);
    }
  }
  return 0;
}