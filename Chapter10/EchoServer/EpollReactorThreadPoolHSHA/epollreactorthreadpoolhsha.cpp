#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "../epollctl.hpp"

std::mutex Mutex;
std::condition_variable Cond;
std::queue<EchoServer::Conn *> Queue;

void pushInQueue(EchoServer::Conn *conn) {
  {
    std::unique_lock<std::mutex> locker(Mutex);
    Queue.push(conn);
  }
  Cond.notify_one();
}

EchoServer::Conn *getQueueData() {
  std::unique_lock<std::mutex> locker(Mutex);
  Cond.wait(locker, []() -> bool { return Queue.size() > 0; });
  EchoServer::Conn *conn = Queue.front();
  Queue.pop();
  return conn;
}

void workerHandler(bool directSend) {
  while (true) {
    EchoServer::Conn *conn = getQueueData();
    conn->EnCode();
    if (directSend) {  // 直接把数据发送给客户端，而不是通过I/O线程来发送
      while (not conn->FinishWrite()) {
        if (not conn->Write(false)) {
          break;
        }
      }
      EchoServer::ClearEvent(conn);
      delete conn;
    } else {
      EchoServer::ModToWriteEvent(conn);  // 监听写事件，数据通过I/O线程来发送
    }
  }
}

void ioHandler(char *argv[]) {
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), true);
  if (sockFd < 0) {
    return;
  }
  epoll_event events[2048];
  int epollFd = epoll_create(1024);
  if (epollFd < 0) {
    perror("epoll_create failed");
    return;
  }
  EchoServer::Conn conn(sockFd, epollFd, true);
  EchoServer::SetNotBlock(sockFd);
  EchoServer::AddReadEvent(&conn);
  int msec = -1;
  while (true) {
    int num = epoll_wait(epollFd, events, 2048, msec);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < num; i++) {
      EchoServer::Conn *conn = (EchoServer::Conn *)events[i].data.ptr;
      if (conn->Fd() == sockFd) {
        EchoServer::LoopAccept(sockFd, 2048, [epollFd](int clientFd) {
          EchoServer::Conn *conn = new EchoServer::Conn(clientFd, epollFd, true);
          EchoServer::SetNotBlock(clientFd);
          EchoServer::AddReadEvent(conn, false, true);  // 监听可读事件，开启oneshot
        });
        continue;
      }
      auto releaseConn = [&conn]() {
        EchoServer::ClearEvent(conn);
        delete conn;
      };
      if (events[i].events & EPOLLIN) {  // 可读
        if (not conn->Read()) {  // 执行非阻塞read
          releaseConn();
          continue;
        }
        if (conn->OneMessage()) {
          pushInQueue(conn);  // 入共享输入队列，有锁
        } else {
          EchoServer::ReStartReadEvent(conn);  // 还没收到完整的请求，则重新启动可读事件的监听，携带oneshot选项
        }
      }
      if (events[i].events & EPOLLOUT) {  // 可写
        if (not conn->Write(false)) {  // 执行非阻塞write
          releaseConn();
          continue;
        }
        if (conn->FinishWrite()) {  // 完成了请求的应答写，则可以释放连接close
          releaseConn();
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./EpollReactorThreadPoolHSHA 0.0.0.0 1688 1" << std::endl;
    return -1;
  }
  bool directSend = (std::string(argv[3]) == "1");
  for (int i = 0; i < EchoServer::GetNProcs(); i++) {  // 创建worker线程
    std::thread(workerHandler, directSend).detach();  // 这里需要调用detach，让创建的线程独立运行
  }
  for (int i = 0; i < EchoServer::GetNProcs(); i++) {  // 创建io线程
    std::thread(ioHandler, argv).detach();  // 这里需要调用detach，让创建的线程独立运行
  }
  while (true) sleep(1);  // 主线程陷入死循环
  return 0;
}