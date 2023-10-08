#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <thread>

#include "../common.hpp"

std::mutex Mutex;

void handlerClient(int clientFd) {
  std::string msg;
  if (not EchoServer::RecvMsg(clientFd, msg)) {
    return;
  }
  EchoServer::SendMsg(clientFd, msg);
  close(clientFd);
}

void handler(int sockFd) {
  while (true) {
    int clientFd;
    // follower等待获取锁，成为leader
    {
      std::lock_guard<std::mutex> guard(Mutex);
      clientFd = accept(sockFd, NULL, 0);  // 获取锁，并获取客户端的连接
      if (clientFd < 0) {
        perror("accept failed");
        continue;
      }
    }
    handlerClient(clientFd);  // 处理每个客户端请求
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./LeaderAndFollower 0.0.0.0 1688" << std::endl;
    return -1;
  }
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), false);
  if (sockFd < 0) {
    return -1;
  }
  for (int i = 0; i < EchoServer::GetNProcs(); i++) {
    std::thread(handler, sockFd).detach();  // 这里需要调用detach，让创建的线程独立运行
  }
  while (true) sleep(1);  // 主进程陷入死循环
  return 0;
}