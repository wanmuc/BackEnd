#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "../common.hpp"

typedef struct Stat {
  int sum{0};
  int success{0};
  int failure{0};
  int spendms{0};
} Stat;

std::mutex Mutex;
Stat FinalStat;

bool getConnection(sockaddr_in &addr, int &sockFd) {
  sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd < 0) {
    perror("socket failed");
    return false;
  }
  int ret = connect(sockFd, (sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("connect failed");
    close(sockFd);
    return false;
  }
  struct linger lin;
  lin.l_onoff = 1;
  lin.l_linger = 0;
  // 设置调用close关闭tcp连接时，直接发送RST包，tcp连接直接复位，进入到closed状态。
  if (0 == setsockopt(sockFd, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin))) {
    return true;
  }
  perror("setsockopt failed");
  close(sockFd);
  return false;
}

int getSpendMs(timeval begin, timeval end) {
  end.tv_sec -= begin.tv_sec;
  end.tv_usec -= begin.tv_usec;
  if (end.tv_usec <= 0) {
    end.tv_sec -= 1;
    end.tv_usec += 1000000;
  }
  return end.tv_sec * 1000 + end.tv_usec / 1000;  //计算运行的时间，单位ms
}

void client(int theadId, Stat *curStat, char *argv[]) {
  int sum = 0;
  int success = 0;
  int failure = 0;
  int spendms = 0;
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(argv[1]));
  addr.sin_addr.s_addr = inet_addr(std::string("127.0.0." + std::to_string(theadId + 1)).c_str());
  int msgLen = atoi(argv[2]) * 1024;
  if (msgLen <= 0) {
    msgLen = 100;  // 最小发送100个字节
  }
  std::string message(msgLen - 4, 'a');
  int concurrency = atoi(argv[3]) / 10;  // 每个线程的并发数
  int *sockFd = new int[concurrency];
  timeval end;
  timeval begin;
  gettimeofday(&begin, NULL);
  for (int i = 0; i < concurrency; i++) {
    if (not getConnection(addr, sockFd[i])) {
      sockFd[i] = 0;
      failure++;
    }
  }
  auto failureDeal = [&sockFd, &failure](int i) {
    close(sockFd[i]);
    sockFd[i] = 0;
    failure++;
  };
  std::cout << "threadId[" << theadId << "] finish connection" << std::endl;
  for (int i = 0; i < concurrency; i++) {
    if (sockFd[i]) {
      if (not EchoServer::SendMsg(sockFd[i], message)) {
        failureDeal(i);
      }
    }
  }
  std::cout << "threadId[" << theadId << "] finish send message" << std::endl;
  for (int i = 0; i < concurrency; i++) {
    if (sockFd[i]) {
      std::string respMessage;
      if (not EchoServer::RecvMsg(sockFd[i], respMessage)) {
        failureDeal(i);
        continue;
      }
      if (respMessage != message) {
        failureDeal(i);
        continue;
      }
      close(sockFd[i]);
      success++;
    }
  }
  delete[] sockFd;
  std::cout << "threadId[" << theadId << "] finish recv message" << std::endl;
  sum = success + failure;
  gettimeofday(&end, NULL);
  spendms = getSpendMs(begin, end);
  std::lock_guard<std::mutex> guard(Mutex);
  curStat->sum += sum;
  curStat->success += success;
  curStat->failure += failure;
  curStat->spendms += spendms;
}

void UpdateFinalStat(Stat stat) {
  FinalStat.sum += stat.sum;
  FinalStat.success += stat.success;
  FinalStat.failure += stat.failure;
  FinalStat.spendms += stat.spendms;
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    std::cout << "invalid input" << std::endl;
    std::cout << "example: ./BenchMark 1688 1 1000 1" << std::endl;
    return -1;
  }
  int runSecond = 1;  // 压测总运行时间，单位秒
  if (atoi(argv[4]) > runSecond) {
    runSecond = atoi(argv[4]);
  }
  timeval end;
  timeval runBeginTime;
  gettimeofday(&runBeginTime, NULL);
  int runRoundCount = 0;
  while (true) {
    Stat curStat;
    std::thread threads[10];
    for (int threadId = 0; threadId < 10; threadId++) {
      threads[threadId] = std::thread(client, threadId, &curStat, argv);
    }
    for (int threadId = 0; threadId < 10; threadId++) {
      threads[threadId].join();
    }
    runRoundCount++;
    curStat.spendms /= 10;
    UpdateFinalStat(curStat);
    gettimeofday(&end, NULL);
    std::cout << "round " << runRoundCount << " spend " << curStat.spendms << " ms. " << std::endl;
    if (getSpendMs(runBeginTime, end) >= runSecond * 1000) {
      break;
    }
    sleep(2);  // 间隔2秒，再发起下一轮压测，这样压测结果更稳定
  }
  std::cout << "total spend " << FinalStat.spendms << " ms. avg spend " << FinalStat.spendms / runRoundCount
            << " ms. sum[" << FinalStat.sum << "],success[" << FinalStat.success << "],failure[" << FinalStat.failure
            << "]" << std::endl;
  return 0;
}