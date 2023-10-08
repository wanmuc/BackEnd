#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "../common.hpp"

void handlerClient(int clientFd) {
  std::string msg;
  if (not EchoServer::RecvMsg(clientFd, msg)) {
    return;
  }
  EchoServer::SendMsg(clientFd, msg);
}

void handler(char* argv[]) {
  // isReusePort设置为true，开启SO_REUSEPORT选项
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), true);
  if (sockFd < 0) {
    return;
  }
  while (true) {
    int clientFd = accept(sockFd, NULL, 0);
    if (clientFd < 0) {
      perror("accept failed");
      continue;
    }
    handlerClient(clientFd);
    close(clientFd);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./ProcessPool2 0.0.0.0 1688" << std::endl;
    return -1;
  }
  for (int i = 0; i < EchoServer::GetNProcs(); i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      continue;
    }
    if (0 == pid) {
      handler(argv);  // 子进程陷入死循环，处理客户端请求
      exit(0);
    }
  }
  while (true) sleep(1);  // 父进程陷入死循环
  return 0;
}