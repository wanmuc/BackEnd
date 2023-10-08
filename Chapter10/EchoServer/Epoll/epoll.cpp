#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "../epollctl.hpp"

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
    std::cout << "example: ./Epoll 0.0.0.0 1688" << std::endl;
    return -1;
  }
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), false);
  if (sockFd < 0) {
    return -1;
  }
  epoll_event events[2048];
  int epollFd = epoll_create(1024);
  if (epollFd < 0) {
    perror("epoll_create failed");
    return -1;
  }
  EchoServer::Conn conn(sockFd, epollFd, false);
  EchoServer::SetNotBlock(sockFd);
  EchoServer::AddReadEvent(&conn);
  while (true) {
    int num = epoll_wait(epollFd, events, 2048, -1);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < num; i++) {
      EchoServer::Conn *conn = (EchoServer::Conn *)events[i].data.ptr;
      if (conn->Fd() == sockFd) {
        EchoServer::LoopAccept(sockFd, 2048, [epollFd](int clientFd) {
          EchoServer::Conn *conn = new EchoServer::Conn(clientFd, epollFd, false);
          EchoServer::AddReadEvent(conn);                 // 监听可读事件，保持fd为阻塞IO
          EchoServer::SetTimeOut(conn->Fd(), 0, 500000);  // 设置读写超时时间为500ms
        });
        continue;
      }
      handlerClient(conn->Fd());
      EchoServer::ClearEvent(conn);
      delete conn;
    }
  }
  return 0;
}
