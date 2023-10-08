#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

typedef void (*signalHanler)(int signo);

const int16_t ICMP_ECHO_TYPE_REQ{8};
const int16_t ICMP_ECHO_TYPE_RESP{0};
const int16_t ICMP_ECHO_CODE{0};
const size_t IP_PROTO_MAX_SIZE{1500};
const size_t ICMP_ECHO_PKT_SIZE{8 + 4 + 4};

bool running{true};
char ipStr[1024]{0};

class PingBase {
 public:
  static void sigHandler(int signum) { running = false; }
  static void signalDeal(int signum, signalHanler handler) {
    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    assert(0 == sigaction(signum, &act, NULL));
  }
  static void signalDealReg() {
    signalDeal(SIGTERM, sigHandler);  // kill进程时，触发的信号
    signalDeal(SIGINT, sigHandler);   // 进程前台运行时，按ctrl + C触发的信号
    signalDeal(SIGQUIT, sigHandler);  // 进程前台运行时，按ctrl + \触发的信号
  }
  static bool getAddrInfo(char *host, struct sockaddr_in *addr) {
    if (NULL == host || NULL == addr) return false;
    in_addr_t inaddr;
    struct hostent *he = NULL;
    bzero(addr, sizeof(sockaddr_in));
    addr->sin_family = AF_INET;

    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE) {
      memcpy(&addr->sin_addr, &inaddr, sizeof(inaddr));
    } else {
      he = gethostbyname(host);
      if (NULL == he) {
        return false;
      } else {
        memcpy(&addr->sin_addr, he->h_addr, he->h_length);
      }
    }
    return true;
  }
  uint16_t getCheckSum(uint8_t *pkt, size_t size) {
    uint32_t sum = 0;
    uint16_t checkSum = 0;
    while (size > 1) {
      sum += (*(uint16_t *)pkt);
      pkt += 2;
      size -= 2;
    }
    if (1 == size) {
      *(uint8_t *)(&checkSum) = *pkt;
      sum += checkSum;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    checkSum = ~sum;
    return checkSum;
  }
  void setPid(pid_t pid) { this->pid = pid; }

 public:
  uint16_t pid{0};
  uint16_t sendCount{0};
  uint16_t recvCount{0};
  double rttMin{0.0};
  double rttMax{0.0};
  vector<double> rtts{};
  struct timeval beginTime;
};

class PingSend : public PingBase {
 public:
  void setIcmpPkt(uint8_t *pkt) {
    uint8_t *checkSum = NULL;
    *pkt = ICMP_ECHO_TYPE_REQ;  //设置类型字段
    ++pkt;
    *pkt = ICMP_ECHO_CODE;  //设置代码字段
    ++pkt;
    checkSum = pkt;
    *(uint16_t *)pkt = 0;  //校验和先设置为0
    pkt += 2;
    *(uint16_t *)pkt = htons(pid);  //设置标识符
    pkt += 2;
    *(uint16_t *)pkt = htons(++sendCount);  //设置序号
    pkt += 2;
    //设置选项数据
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *(uint32_t *)pkt = htonl((uint32_t)tv.tv_sec);  //设置秒
    pkt += 4;
    *(uint32_t *)pkt = htonl(tv.tv_usec);  //设置微妙
    //重新设置校验和
    *(uint16_t *)checkSum = getCheckSum(checkSum - 2, ICMP_ECHO_PKT_SIZE);
  }
  void run(struct sockaddr_in *addr, int32_t wFd) {
    int sockFd = 0;
    uint8_t pkt[ICMP_ECHO_PKT_SIZE];
    sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockFd < 0) {
      perror("call socket failed!");
      write(wFd, &sendCount, sizeof(sendCount));
      return;
    }
    while (running) {
      setIcmpPkt(pkt);
      sendto(sockFd, pkt, ICMP_ECHO_PKT_SIZE, 0, (struct sockaddr *)addr, (socklen_t)sizeof(*addr));
      sleep(1);
    }
    write(wFd, &sendCount, sizeof(sendCount));
  }
};

class PingRecv : public PingBase {
 public:
  void respStat(double rtt) {
    if (rtts.size() <= 0) {
      rttMin = rtt;
      rttMax = rtt;
    }
    rttMin = rtt < rttMin ? rtt : rttMin;
    rttMax = rtt > rttMax ? rtt : rttMax;
    rtts.push_back(rtt);
  }
  double getIntervalMs(struct timeval begin, struct timeval end) {
    if ((end.tv_usec -= begin.tv_usec) < 0) {
      end.tv_usec += 1000000;
      end.tv_sec -= 1;
    }
    end.tv_sec -= begin.tv_sec;
    return end.tv_sec * 1000.0 + end.tv_usec / 1000.0;
  }
  double getRtt(uint8_t *icmpOpt) {
    struct timeval current;
    struct timeval reqSendTime;
    gettimeofday(&current, NULL);
    reqSendTime.tv_sec = ntohl(*(uint32_t *)icmpOpt);
    reqSendTime.tv_usec = ntohl(*(uint32_t *)(icmpOpt + 4));
    return getIntervalMs(reqSendTime, current);
  }
  double getTotal() {
    struct timeval current;
    gettimeofday(&current, NULL);
    return getIntervalMs(beginTime, current);
  }
  void dealResp(uint8_t *pkt, ssize_t len) {
    if (len <= 0) return;
    //取IP首部长度
    ssize_t ipHeaderLen = ((*pkt) & 0x0f) << 2;
    //判断IP数据报的协议字段是否为ICMP包
    if (IPPROTO_ICMP != *(pkt + 9)) return;
    //校验ICMP报文长度
    if (len - ipHeaderLen != ICMP_ECHO_PKT_SIZE) return;
    uint8_t *pIcmp = pkt + ipHeaderLen;
    if (*pIcmp != ICMP_ECHO_TYPE_RESP) return;
    uint16_t tempPid = ntohs(*(uint16_t *)(pIcmp + 4));
    uint16_t sendId = ntohs(*(uint16_t *)(pIcmp + 6));
    if (tempPid != pid) return;
    uint8_t ttl = *(pkt + 8);
    double rtt = getRtt(pIcmp + 8);
    respStat(rtt);  //统计icmp应答
    printf("%d bytes from %s: icmp_seq=%u, ttl=%u, rtt=%.3f ms\n", ICMP_ECHO_PKT_SIZE, ipStr, sendId, ttl, rtt);
  }
  void printReport() {
    double sum = std::accumulate(rtts.begin(), rtts.end(), 0.0);
    double avg = sum / rtts.size();
    double loss = 0;
    if (sendCount > 0) {
      loss = (sendCount - (uint16_t)rtts.size()) / (double)sendCount;
      loss *= 100;
    }
    int64_t totalTime = (int64_t)getTotal();
    printf("\n-- %s ping statistics ---\n", ipStr);
    printf("%u packets transmitted, %u received, %.2f%% packet loss, time %ldms\n", sendCount, rtts.size(), loss,
           totalTime);
    if (rtts.size() > 0) {
      printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n", rttMin, avg, rttMax);
    }
  }
  void run(struct sockaddr_in *addr, int32_t rFd) {
    int sockFd = 0;
    uint8_t pkt[IP_PROTO_MAX_SIZE];
    socklen_t len = sizeof(*addr);
    gettimeofday(&beginTime, NULL);
    sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    while (running) {
      ssize_t n = recvfrom(sockFd, pkt, IP_PROTO_MAX_SIZE, 0, (struct sockaddr *)addr, (socklen_t *)&len);
      if (n < 0) {
        if (EINTR == errno) {  //调用被中断则继续
          continue;
        } else {
          perror("call recvfrom failed!");
        }
      }
      dealResp(pkt, n);  //这里收到的包为IP数据报，包含IP数据报的首部
    }
    read(rFd, &sendCount, sizeof(sendCount));
    printReport();
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "param invalid!" << endl;
    cout << "Usage: myping www.baidu.com" << endl;
    return -1;
  }

  int fd[2];
  int ret = 0;
  struct sockaddr_in addr;
  pid_t childPid = 0;

  if (!PingBase::getAddrInfo(argv[1], &addr)) {
    cout << "myping: unknown host " << argv[1] << endl;
    return -1;
  }

  ret = pipe(fd);  //创建匿名管道用于父子进程间通信
  if (ret != 0) {
    cout << "call pipe() failed! error msg:" << strerror(errno) << endl;
    return -1;
  }
  inet_ntop(AF_INET, &addr.sin_addr, ipStr, 1024);
  cout << "ping " << argv[1] << " (" << ipStr << ") " << endl;

  childPid = fork();  //创建子进程
  if (childPid < 0) {
    cout << "call fork() failed! error msg:" << strerror(errno) << endl;
    return -1;
  }
  PingBase::signalDealReg();

  if (0 == childPid) {  //子进程
    close(fd[0]);       //关闭匿名管道读端
    PingSend pingSend;
    pingSend.setPid(getpid() & 0xffff);  // ICMP的标识符只有16位，故这里只取子进程的pid的低16位
    pingSend.run(&addr, fd[1]);          //子进程用于发送icmp回显请求
  } else {                               //父进程
    close(fd[1]);                        //关闭匿名管道写端
    PingRecv pingRecv;
    pingRecv.setPid(childPid & 0xffff);  // ICMP的标识符只有16位，故这里只取子进程的pid的低16位
    pingRecv.run(&addr, fd[0]);          //父进程用于接收icmp回显应答
  }
  return 0;
}