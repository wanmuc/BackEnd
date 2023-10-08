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

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./MultiThread 0.0.0.0 1688" << std::endl;
    return -1;
  }
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), false);
  if (sockFd < 0) {
    return -1;
  }
  while (true) {
    int clientFd = accept(sockFd, NULL, 0);
    if (clientFd < 0) {
      perror("accept failed");
      continue;
    }
    std::thread(handlerClient, clientFd).detach();  // 这里需要调用detach，让创建的线程独立运行
  }
  return 0;
}