#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

#include "../coroutine.h"
#include "../epollctl.hpp"

int SubEpollFd;
std::mutex Mutex;
std::condition_variable Cond;
bool SubReactorStart{false};

struct EventData {
  EventData(int fd, int epoll_fd) : fd_(fd), epoll_fd_(epoll_fd){};
  int fd_{0};
  int epoll_fd_{0};
  int cid_{MyCoroutine::INVALID_ROUTINE_ID};
  MyCoroutine::Schedule *schedule_{nullptr};
};

enum DecodeStatus {
  HEAD = 1,  // 解析协议头（协议体长度）
  BODY = 2,  // 解析协议体
};

class Packet {
 public:
  Packet() = default;
  ~Packet() {
    if (data_) {
      free(data_);
    }
    len_ = 0;
  }
  void Alloc(size_t size) {
    if (data_) {
      free(data_);
    }
    len_ = size;
    data_ = (uint8_t *)malloc(len_);
  }
  void ReAlloc(ssize_t len) {
    data_ = (uint8_t *)realloc(data_, len);
    len_ = len;
  }
  uint8_t *Data() { return data_; }
  ssize_t Len() { return len_; }

 public:
  uint8_t *data_{nullptr};  // 二进制缓冲区
  ssize_t len_{0};          // 缓冲区的长度
};

class Codec {
 public:
  Codec() {
    pkt_.Alloc(4);  // 缓冲区第一次只分配协议头大小的空间
  }
  uint8_t *Data() { return pkt_.Data() + pkt_buf_len_; }
  size_t Len() {
    if (0 == body_len_) {       // 还没解析到头之前
      return 4 - pkt_buf_len_;  // 要读满4个字节才能解析出body的长度
    }
    return 4 + body_len_ - pkt_buf_len_;  // body剩余要读取的数据长度
  }

  void EnCode(const std::string &msg, Packet &pkt) {
    pkt.Alloc(msg.length() + 4);
    *(uint32_t *)pkt.Data() = htonl(msg.length());  // 协议体长度转换成网络字节序
    memmove(pkt.Data() + 4, msg.data(), msg.length());
  }
  void DeCode(size_t len) {
    pkt_buf_len_ += len;
    uint32_t decodeLen = 0;                           // 本次解析的字节长度
    uint32_t curLen = pkt_buf_len_ - pkt_parse_len_;  // 还有多少字节需要解析
    uint8_t *data = pkt_.Data() + pkt_parse_len_;
    while (curLen > 0) {  // 只要还有未解析的网络字节流，就持续解析
      bool decodeBreak = false;
      if (HEAD == decode_status_) {  // 解析协议头
        decodeHead(&data, curLen, decodeLen, decodeBreak);
        if (decodeBreak) {
          break;
        }
      }
      if (BODY == decode_status_) {  // 解析完协议头，解析协议体
        decodeBody(&data, curLen, decodeLen, decodeBreak);
        if (decodeBreak) {
          break;
        }
      }
    }
    if (decodeLen > 0) {  // 更新解析完的数据长度
      pkt_parse_len_ += decodeLen;
    }
  }
  bool GetMessage(std::string &msg) {
    if (msg_list_.empty()) {
      return false;
    }
    msg = msg_list_.front();
    msg_list_.pop_front();
    return true;
  }

 private:
  bool decodeHead(uint8_t **data, uint32_t &curLen, uint32_t &decodeLen, bool &decodeBreak) {
    if (curLen < 4) {  // head固定4个字节
      decodeBreak = true;
      return true;
    }
    body_len_ = ntohl(*(uint32_t *)(*data));
    curLen -= 4;
    (*data) += 4;
    decodeLen += 4;
    decode_status_ = BODY;
    pkt_.ReAlloc(4 + body_len_);  // 重新分配内存空间
    return true;
  }
  bool decodeBody(uint8_t **data, uint32_t &curLen, uint32_t &decodeLen, bool &decodeBreak) {
    if (curLen < body_len_) {
      decodeBreak = true;
      return true;
    }
    std::string msg;
    msg.append((const char *)*data, body_len_);
    curLen -= body_len_;
    (*data) += body_len_;
    decodeLen += body_len_;
    msg_list_.push_back(msg);
    decode_status_ = HEAD;
    body_len_ = 0;
    return true;
  }

 private:
  DecodeStatus decode_status_{HEAD};  // 当前解析状态
  Packet pkt_;                        // 网络字节流缓冲区
  size_t pkt_parse_len_{0};           // pkt缓冲区中被解析的长度
  size_t pkt_buf_len_{0};             // 缓冲区中已经缓冲的数据长度
  uint32_t body_len_{0};              // 当前消息的协议体长度
  std::list<std::string> msg_list_;   // 解析好的消息列表
};

void EchoDeal(const std::string reqMessage, std::string &respMessage) { respMessage = reqMessage; }

void handlerClient(void *arg) {
  EventData *eventData = (EventData *)arg;
  auto releaseConn = [&eventData]() {
    EchoServer::ClearEvent(eventData->epoll_fd_, eventData->fd_);
    delete eventData;  // 释放内存
  };
  ssize_t ret = 0;
  Codec codec;
  std::string reqMessage;
  std::string respMessage;
  while (true) {                                            // 读操作
    ret = read(eventData->fd_, codec.Data(), codec.Len());  // 一次最多读取100字节
    if (ret == 0) {
      perror("peer close connection");
      releaseConn();
      return;
    }
    if (ret < 0) {
      if (errno == EINTR or errno == EAGAIN or errno == EWOULDBLOCK) {
        MyCoroutine::CoroutineYield(*eventData->schedule_);  // 让出cpu，切换到主协程，等待下一次数据可读
      } else {
        perror("read failed");
        releaseConn();
        return;
      }
    } else {
      codec.DeCode(ret);                   // 解析请求数据
      if (codec.GetMessage(reqMessage)) {  // 解析出一个完整的请求
        break;
      }
    }
  }
  // 执行到这里说明已经读取到一个完整的请求
  EchoDeal(reqMessage, respMessage);  // 业务handler的封装，这样协程的调用就对业务逻辑函数EchoDeal透明，
  Packet pkt;
  codec.EnCode(respMessage, pkt);
  bool writeFirstBlock = false;
  ssize_t sendLen = 0;
  while (sendLen != pkt.Len()) {  // 写操作
    ret = write(eventData->fd_, pkt.Data() + sendLen, pkt.Len() - sendLen);
    if (ret < 0) {
      if (errno == EINTR or errno == EAGAIN or errno == EWOULDBLOCK) {
        if (not writeFirstBlock) {
          writeFirstBlock = true;
          EchoServer::ModToWriteEvent(eventData->epoll_fd_, eventData->fd_, eventData);  // 监听可写事件。
        }
        MyCoroutine::CoroutineYield(*eventData->schedule_);  // 让出cpu，切换到主协程，等待下一次数据可写
      } else {
        perror("write failed");
        releaseConn();
        return;
      }
    } else {
      sendLen += ret;
    }
  }
  releaseConn();
}

void waitSubReactor() {
  std::unique_lock<std::mutex> locker(Mutex);
  Cond.wait(locker, []() -> bool { return SubReactorStart; });
}

void subReactorNotifyReady() {
  {
    std::unique_lock<std::mutex> locker(Mutex);
    SubReactorStart = true;
  }
  Cond.notify_one();
}

void mainHandler(char *argv[]) {
  waitSubReactor();  // 等待subReactor协程都启动完毕
  int sockFd = EchoServer::CreateListenSocket(argv[1], atoi(argv[2]), true);
  if (sockFd < 0) {
    return;
  }
  epoll_event events[1024];
  int mainEpollFd = epoll_create(1024);
  if (mainEpollFd < 0) {
    perror("epoll_create failed");
    return;
  }
  EventData eventData(sockFd, mainEpollFd);
  EchoServer::SetNotBlock(sockFd);
  EchoServer::AddReadEvent(mainEpollFd, sockFd, &eventData);
  while (true) {
    int num = epoll_wait(mainEpollFd, events, 1024, -1);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    }
    // 执行到这里就是有客户端的连接到来了
    EchoServer::LoopAccept(sockFd, 2048, [](int clientFd) {
      EventData *eventData = new EventData(clientFd, SubEpollFd);
      EchoServer::SetNotBlock(clientFd);
      EchoServer::AddReadEvent(SubEpollFd, clientFd, eventData);  // 监听可读事件，添加到subReactor线程中
    });
  }
}

void subHandler() {
  epoll_event events[2048];
  SubEpollFd = epoll_create(1024);
  if (SubEpollFd < 0) {
    perror("epoll_create failed");
    return;
  }
  subReactorNotifyReady();
  MyCoroutine::Schedule schedule;
  MyCoroutine::ScheduleInit(schedule, 10000);  // 协程池初始化
  int msec = -1;
  while (true) {
    int num = epoll_wait(SubEpollFd, events, 2048, msec);
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

void handler(char *argv[]) {
  std::thread(mainHandler, argv).detach();  // 启动mainReactor,这里需要调用detach，让创建的线程独立运行
  subHandler();                             // 启动subReactor
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./Mixed 0.0.0.0 1688" << std::endl;
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