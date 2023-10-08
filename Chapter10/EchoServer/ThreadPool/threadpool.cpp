#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "../common.hpp"

void handlerClient(int clientFd) {
  std::string msg;
  if (not EchoServer::RecvMsg(clientFd, msg)) {
    return;
  }
  EchoServer::SendMsg(clientFd, msg);
  close(clientFd);
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
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./ThreadPool 0.0.0.0 1688" << std::endl;
    return -1;
  }
  for (int i = 0; i < EchoServer::GetNProcs(); i++) {
    std::thread(handler, argv).detach();  // 这里需要调用detach，让创建的线程独立运行
  }
  while (true) sleep(1);  // 主线程陷入死循环
  return 0;
}