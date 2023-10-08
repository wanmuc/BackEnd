#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <unordered_set>

#include "../common.hpp"

void updateReadSet(std::unordered_set<int> &clientFds, int &maxFd, int sockFd, fd_set &readSet) {
  maxFd = sockFd;
  FD_ZERO(&readSet);
  FD_SET(sockFd, &readSet);
  for (const auto &clientFd : clientFds) {
    if (clientFd > maxFd) {
      maxFd = clientFd;
    }
    FD_SET(clientFd, &readSet);
  }
}

void handlerClient(int clientFd) {
  std::string msg;
  if (not EchoServer::RecvMsg(clientFd, msg)) {
    return;
  }
  EchoServer::SendMsg(clientFd, msg);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./Select 0.0.0.0 1688" << std::endl;
    return -1;
  }
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), false);
  if (sockFd < 0) {
    return -1;
  }
  int maxFd;
  fd_set readSet;
  EchoServer::SetNotBlock(sockFd);
  std::unordered_set<int> clientFds;
  while (true) {
    updateReadSet(clientFds, maxFd, sockFd, readSet);
    int ret = select(maxFd + 1, &readSet, NULL, NULL, NULL);
    if (ret <= 0) {
      if (ret < 0) perror("select failed");
      continue;
    }
    for (int i = 0; i <= maxFd; i++) {
      if (not FD_ISSET(i, &readSet)) {
        continue;
      }
      if (i == sockFd) {  // 监听的sockFd可读，则表示有新的链接
        EchoServer::LoopAccept(sockFd, 1024, [&clientFds](int clientFd) {
          clientFds.insert(clientFd);  // 新增到要监听的fd集合中
        });
        continue;
      }
      handlerClient(i);
      clientFds.erase(i);
      close(i);
    }
  }
  return 0;
}
