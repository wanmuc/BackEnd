#include <signal.h>
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
  close(clientFd);
}

void childExitSignalHandler() {
  struct sigaction act;
  act.sa_handler = SIG_IGN;   //设置信号处理函数，这里忽略子进程的退出信号
  sigemptyset(&act.sa_mask);  //信号屏蔽设置为空
  act.sa_flags = 0;           //标志位设置为0
  sigaction(SIGCHLD, &act, NULL);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./MultiProcess 0.0.0.0 1688" << std::endl;
    return -1;
  }
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), false);
  if (sockFd < 0) {
    return -1;
  }
  childExitSignalHandler();  // 这里需要忽略子进程退出信号，否则会导致大量的僵尸进程，服务后续无法再创建子进程
  while (true) {
    int clientFd = accept(sockFd, NULL, 0);
    if (clientFd < 0) {
      perror("accept failed");
      continue;
    }
    pid_t pid = fork();
    if (pid == -1) {
      perror("fork failed");
      continue;
    }
    if (pid == 0) {  // 子进程
      handlerClient(clientFd);
      exit(0);  // 处理完请求，子进程直接退出
    } else {
      close(clientFd);  // 父进程直接关闭客户端连接，否则文件描述符会泄露
    }
  }
  return 0;
}