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

#include "../coroutine.h"
#include "../epollctl.hpp"

struct EventData {
  EventData(int fd, int epoll_fd) : fd_(fd), epoll_fd_(epoll_fd){};
  int fd_{0};
  int epoll_fd_{0};
  int cid_{MyCoroutine::INVALID_ROUTINE_ID};
  MyCoroutine::Schedule *schedule_{nullptr};
};

void EchoDeal(const std::string reqMessage, std::string &respMessage) { respMessage = reqMessage; }

void handlerClient(void *arg) {
  EventData *eventData = (EventData *)arg;
  auto releaseConn = [&eventData]() {
    EchoServer::ClearEvent(eventData->epoll_fd_, eventData->fd_);
    delete eventData;  // 释放内存
  };
  ssize_t ret = 0;
  EchoServer::Codec codec;
  std::string reqMessage;
  std::string respMessage;
  while (true) {  // 读操作
    uint8_t data[100];
    ret = read(eventData->fd_, data, 100);  // 一次最多读取100字节
    if (ret == 0) {
      perror("peer close connection");
      releaseConn();
      return;
    }
    if (ret < 0) {
      if (EINTR == errno) continue;  // 被中断，可以重启读操作
      if (EAGAIN == errno or EWOULDBLOCK == errno) {
        MyCoroutine::CoroutineYield(*eventData->schedule_);  // 让出cpu，切换到主协程，等待下一次数据可读
        continue;
      }
      perror("read failed");
      releaseConn();
      return;
    }
    codec.DeCode(data, ret);             // 解析请求数据
    if (codec.GetMessage(reqMessage)) {  // 解析出一个完整的请求
      break;
    }
  }
  // 执行到这里说明已经读取到一个完整的请求
  EchoDeal(reqMessage, respMessage);  // 业务handler的封装，这样协程的调用就对业务逻辑函数EchoDeal透明，
  EchoServer::Packet pkt;
  codec.EnCode(respMessage, pkt);
  EchoServer::ModToWriteEvent(eventData->epoll_fd_, eventData->fd_, eventData);  // 监听可写事件。
  ssize_t sendLen = 0;
  while (sendLen != pkt.Len()) {  // 写操作
    ret = write(eventData->fd_, pkt.Data() + sendLen, pkt.Len() - sendLen);
    if (ret < 0) {
      if (EINTR == errno) continue;  // 被中断，可以重启写操作
      if (EAGAIN == errno or EWOULDBLOCK == errno) {
        MyCoroutine::CoroutineYield(*eventData->schedule_);  // 让出cpu，切换到主协程，等待下一次数据可写
        continue;
      }
      perror("write failed");
      releaseConn();
      return;
    }
    sendLen += ret;
  }
  releaseConn();
}

void handler(char *argv[]) {
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
  EventData eventData(sockFd, epollFd);
  EchoServer::SetNotBlock(sockFd);
  EchoServer::AddReadEvent(epollFd, sockFd, &eventData);
  MyCoroutine::Schedule schedule;
  MyCoroutine::ScheduleInit(schedule, 10000);  // 协程池初始化
  int msec = -1;
  while (true) {
    int num = epoll_wait(epollFd, events, 2048, msec);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    } else if (num == 0) {  // 没有事件了，下次调用epoll_wait大概率被挂起
      sleep(0);  // 这里直接sleep(0)让出cpu，大概率被挂起，这里主动让出cpu，可以减少一次epoll_wait的调用
      msec = -1;  // 大概率被挂起，故这里超时时间设置为-1
      continue;
    }
    msec = 0;  // 下次大概率还有事件，故msec设置为0
    for (int i = 0; i < num; i++) {
      EventData *eventData = (EventData *)events[i].data.ptr;
      if (eventData->fd_ == sockFd) {
        EchoServer::LoopAccept(sockFd, 2048, [epollFd](int clientFd) {
          EventData *eventData = new EventData(clientFd, epollFd);
          EchoServer::SetNotBlock(clientFd);
          EchoServer::AddReadEvent(epollFd, clientFd, eventData);  // 监听可读事件
        });
        continue;
      }
      if (eventData->cid_ == MyCoroutine::INVALID_ROUTINE_ID) {  // 第一次事件，则创建协程
        if (MyCoroutine::CoroutineCanCreate(schedule)) {
          eventData->schedule_ = &schedule;
          eventData->cid_ = MyCoroutine::CoroutineCreate(schedule, handlerClient, eventData, 0);  // 创建协程
          MyCoroutine::CoroutineResumeById(schedule, eventData->cid_);  // 唤醒刚刚创建的协程处理客户端请求
        } else {
          std::cout << "MyCoroutine is full" << std::endl;
        }
      } else {
        MyCoroutine::CoroutineResumeById(schedule, eventData->cid_);  // 唤醒之前主动让出cpu的协程
      }
    }
    MyCoroutine::ScheduleTryReleaseMemory(schedule);  // 尝试释放内存
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./EpollReactorProcessPoolCoroutine 0.0.0.0 1688" << std::endl;
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