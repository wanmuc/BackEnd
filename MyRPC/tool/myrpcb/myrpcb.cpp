#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "../../common/cmdline.h"
#include "../../common/robustio.hpp"
#include "../../core/routeinfo.hpp"
#include "../../protocol/mixedcodec.hpp"

using namespace std;

#define GREEN_BEGIN "\033[32m"
#define RED_BEGIN "\033[31m"
#define COLOR_END "\033[0m"

string serviceName;
string rpcName;
string jsonStr;
bool byAccessService;
bool oneway;
bool fastResp;
int64_t sleepTime = 2;  // 默认sleep 2秒
int64_t totalTime;
int64_t concurrency;

typedef struct Stat {
  int sum{0};
  int success{0};
  int failure{0};
  int spendms{0};
} Stat;

std::mutex Mutex;
Stat FinalStat;

int createSockAndConnect(Core::Route route) {
  string error;
  int sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd < 0) {
    error = strerror(errno);
    cout << RED_BEGIN << "create socket failed. " << error << COLOR_END << endl;
    return -1;
  }
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(int16_t(route.port_));
  addr.sin_addr.s_addr = inet_addr(route.ip_.c_str());
  int ret = connect(sockFd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret) {
    error = strerror(errno);
    cout << RED_BEGIN << "connect failed. " << error << COLOR_END << endl;
    return -1;
  }
  struct linger lin;
  lin.l_onoff = 1;
  lin.l_linger = 0;
  // 设置调用close关闭tcp连接时，直接发送RST包，tcp连接直接复位，进入到closed状态。
  if (setsockopt(sockFd, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin)) != 0) {
    return -1;
  }
  return sockFd;
}

Core::Route getRoute(int index) {
  Core::Route route;
  Core::TimeOut timeOut;
  Core::RouteInfo routeInfo;
  if (byAccessService) {
    if (not routeInfo.GetRoute("access", route, timeOut, index)) {
      cout << RED_BEGIN << "can't get service_name[" << serviceName << "] route info" << COLOR_END << endl;
      exit(-1);
    }
  } else {
    if (not routeInfo.GetRoute(serviceName, route, timeOut, index)) {
      cout << RED_BEGIN << "can't get service_name[" << serviceName << "] route info" << COLOR_END << endl;
      exit(-1);
    }
  }
  return route;
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

void UpdateFinalStat(Stat stat) {
  FinalStat.sum += stat.sum;
  FinalStat.success += stat.success;
  FinalStat.failure += stat.failure;
  FinalStat.spendms += stat.spendms;
}

bool SendRequest(int sockFd) {
  Protocol::Packet pkt;
  Protocol::MySvrCodec codec;
  Protocol::MySvrMessage reqMessage;
  Protocol::MixedCodec::JsonStrSerializeToMySvr(serviceName, rpcName, jsonStr, reqMessage);
  if (oneway) {
    reqMessage.EnableOneway();  // 开启oneway的flag
  }
  if (not oneway && fastResp) {
    reqMessage.EnableFastResp();  // 开启fast-resp的flag
  }
  codec.Encode(&reqMessage, pkt);
  Common::RobustIo robustIo(sockFd);
  if (robustIo.Write(pkt.DataRaw(), pkt.UseLen()) != (ssize_t)pkt.UseLen()) {
    cout << RED_BEGIN << "send request failed." << COLOR_END << endl;
    return false;
  }
  return true;
}

bool RecvResponse(int sockFd) {
  void *message;
  Protocol::MySvrCodec codec;
  Common::RobustIo robustIo(sockFd);
  while (true) {
    if (robustIo.Read(codec.Data(), codec.Len()) != (ssize_t)codec.Len()) {
      cout << RED_BEGIN << strerror(errno) << COLOR_END << endl;
      cout << RED_BEGIN << "recv response failed." << COLOR_END << endl;
      return false;
    }
    if (not codec.Decode(codec.Len())) {
      cout << RED_BEGIN << "decode response failed." << COLOR_END << endl;
      return false;
    }
    message = codec.GetMessage();
    if (message) {
      break;
    }
  }
  Protocol::MySvrMessage *respMessage = (Protocol::MySvrMessage *)message;
  if (respMessage->context_.status_code() != 0) {
    cout << RED_BEGIN << "response.status_code = " << respMessage->context_.status_code() << COLOR_END << endl;
    delete respMessage;
    return false;
  }
  delete respMessage;
  return true;
}

void client(int theadId, Stat *curStat, Core::Route route) {
  int sum = 0;
  int success = 0;
  int failure = 0;
  int spendms = 0;

  int concurrencyPerThread = concurrency / 10;  // 每个线程的并发数
  if (concurrencyPerThread <= 0) concurrencyPerThread = 1;
  int *sockFd = new int[concurrencyPerThread];
  timeval end;
  timeval begin;
  gettimeofday(&begin, NULL);
  for (int i = 0; i < concurrencyPerThread; i++) {
    sockFd[i] = createSockAndConnect(route);
    if (sockFd[i] < 0) {
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
  for (int i = 0; i < concurrencyPerThread; i++) {
    if (sockFd[i]) {
      if (not SendRequest(sockFd[i])) {
        failureDeal(i);
      }
    }
  }
  std::cout << "threadId[" << theadId << "] finish send message" << std::endl;
  for (int i = 0; i < concurrencyPerThread; i++) {
    if (sockFd[i]) {
      if (oneway) {
        close(sockFd[i]);
        success++;
        continue;
      }
      if (not RecvResponse(sockFd[i])) {
        failureDeal(i);
        continue;
      }
      close(sockFd[i]);
      success++;
    }
  }
  if (not oneway) {
    std::cout << "threadId[" << theadId << "] finish recv message" << std::endl;
  }
  delete[] sockFd;
  sum = success + failure;
  gettimeofday(&end, NULL);
  spendms = getSpendMs(begin, end);
  std::lock_guard<std::mutex> guard(Mutex);
  curStat->sum += sum;
  curStat->success += success;
  curStat->failure += failure;
  curStat->spendms += spendms;
}

void usage() {
  cout << "myrpcb -service_name Echo -rpc_name EchoMySelf -json "
       << "'{\"message\":\"hello\"}' -t 10 -c 1000 [-a -o -f]" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help     print usage" << endl;
  cout << "    -service_name service name" << endl;
  cout << "    -rpc_name     rpc name" << endl;
  cout << "    -json         json message" << endl;
  cout << "    -s            sleep time(unit second)" << endl;
  cout << "    -t            benchmark total time" << endl;
  cout << "    -c            benchmark concurrency" << endl;
  cout << "    -a            by access service" << endl;
  cout << "    -o            is oneway message mode" << endl;
  cout << "    -f            is fast response message mode" << endl;
}

void execBenchMark() {
  timeval end;
  timeval begin;
  gettimeofday(&begin, NULL);
  int runRoundCount = 0;
  while (true) {
    Stat curStat;
    std::thread threads[10];
    for (int threadId = 0; threadId < 10; threadId++) {
      Core::Route route = getRoute(threadId + 1);
      threads[threadId] = std::thread(client, threadId, &curStat, route);
    }
    for (int threadId = 0; threadId < 10; threadId++) {
      threads[threadId].join();
    }
    runRoundCount++;
    curStat.spendms /= 10;
    UpdateFinalStat(curStat);
    gettimeofday(&end, NULL);
    std::cout << "round " << runRoundCount << " spend " << curStat.spendms << " ms. " << std::endl;
    if (getSpendMs(begin, end) >= totalTime * 1000) {
      break;
    }
    sleep(sleepTime);  // 间隔一段时间，再发起下一轮压测，这样压测结果更稳定
  }
  std::cout << "total spend " << FinalStat.spendms << " ms. avg spend " << FinalStat.spendms / runRoundCount
            << " ms. sum[" << FinalStat.sum << "],success[" << FinalStat.success << "],failure[" << FinalStat.failure
            << "]" << std::endl;
}

int main(int argc, char *argv[]) {
  Common::CmdLine::StrOptRequired(&serviceName, "service_name");
  Common::CmdLine::StrOptRequired(&rpcName, "rpc_name");
  Common::CmdLine::StrOptRequired(&jsonStr, "json");
  Common::CmdLine::Int64Opt(&totalTime, "t", 1);
  Common::CmdLine::Int64Opt(&concurrency, "c", 1);
  Common::CmdLine::Int64Opt(&sleepTime, "s", 2);
  Common::CmdLine::BoolOpt(&byAccessService, "a");
  Common::CmdLine::BoolOpt(&oneway, "o");
  Common::CmdLine::BoolOpt(&fastResp, "f");
  Common::CmdLine::SetUsage(usage);
  Common::CmdLine::Parse(argc, argv);
  execBenchMark();
  return 0;
}