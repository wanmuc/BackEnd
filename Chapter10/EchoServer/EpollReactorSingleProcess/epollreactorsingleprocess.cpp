#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "../epollctl.hpp"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./EpollReactorSingleProcess 0.0.0.0 1688 1" << std::endl;
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
  bool isMultiIo = (std::string(argv[3]) == "1");
  EchoServer::Conn conn(sockFd, epollFd, isMultiIo);
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
        EchoServer::LoopAccept(sockFd, 2048, [epollFd, isMultiIo](int clientFd) {
          EchoServer::Conn *conn = new EchoServer::Conn(clientFd, epollFd, isMultiIo);
          EchoServer::SetNotBlock(clientFd);
          EchoServer::AddReadEvent(conn);  // 监听可读事件
        });
        continue;
      }
      auto releaseConn = [&conn]() {
        EchoServer::ClearEvent(conn);
        delete conn;
      };
      if (events[i].events & EPOLLIN) {  // 可读
        if (not conn->Read()) {          // 执行读失败
          releaseConn();
          continue;
        }
        if (conn->OneMessage()) {             // 判断是否要触发写事件
          EchoServer::ModToWriteEvent(conn);  // 修改成只监控可写事件
        }
      }
      if (events[i].events & EPOLLOUT) {  // 可写
        if (not conn->Write()) {          // 执行写失败
          releaseConn();
          continue;
        }
        if (conn->FinishWrite()) {  // 完成了请求的应答写，则可以释放连接
          releaseConn();
        }
      }
    }
  }
  return 0;
}