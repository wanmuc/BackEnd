#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
using namespace std;

const int16_t ICMP_ECHO_TYPE_REQ{8};
const int16_t ICMP_ECHO_TYPE_RESP{0};
const int16_t ICMP_ECHO_CODE{0};
const int16_t ICMP_TIME_OUT_TYPE{11};
const int16_t ICMP_TIME_OUT_INTRANS_CODE{0};
const size_t ICMP_ECHO_PKT_SIZE{8 + 4 + 4};
const size_t IP_PROTO_MAX_SIZE{1500};

class TraceRoute {
 public:
  bool init(char *host) {
    if (NULL == host) return false;
    in_addr_t inaddr;
    struct hostent *he = NULL;
    bzero(&addr, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;

    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE) {
      memcpy(&addr.sin_addr, &inaddr, sizeof(inaddr));
    } else {
      he = gethostbyname(host);
      if (NULL == he) {
        return false;
      } else {
        memcpy(&addr.sin_addr, he->h_addr, he->h_length);
      }
    }
    sinAddrSize = sizeof(addr.sin_addr);
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
  double getRtt() { return getIntervalMs(sendTime, recvTime); }
  bool checkSelfPkt(uint8_t *icmp) {
    uint16_t pid = ntohs(*(uint16_t *)(icmp + 4));
    uint16_t seq = ntohs(*(uint16_t *)(icmp + 6));
    if (this->pid == pid && this->seq == seq) {
      return true;
    }
    return false;
  }
  void setIcmpPkt(uint8_t *pkt, int16_t seq) {
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
    *(uint16_t *)pkt = htons(seq);  //设置序号
    pkt += 2;
    //设置选项数据
    gettimeofday(&sendTime, NULL);
    *(uint32_t *)pkt = htonl((uint32_t)sendTime.tv_sec);  //设置秒
    pkt += 4;
    *(uint32_t *)pkt = htonl(sendTime.tv_usec);  //设置微妙
    //重新设置校验和
    *(uint16_t *)checkSum = getCheckSum(checkSum - 2, ICMP_ECHO_PKT_SIZE);
  }

  void sendIcmpEchoReq(int sockFd, int16_t seq, int ttl) {
    uint8_t pkt[ICMP_ECHO_PKT_SIZE];
    setIcmpPkt(pkt, seq);
    setsockopt(sockFd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    sendto(sockFd, pkt, ICMP_ECHO_PKT_SIZE, 0, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
  }

  void dealResp(int recvFd, uint8_t *pkt, ssize_t len, bool &ifBreak, bool &done) {
    if (len <= 0) return;
    //取IP首部长度
    ssize_t ipHeaderLen = ((*pkt) & 0x0f) << 2;
    //判断IP数据报的协议字段是否为ICMP包
    if (IPPROTO_ICMP != *(pkt + 9)) return;
    //校验ICMP报文长度
    if (len - ipHeaderLen < ICMP_ECHO_PKT_SIZE) return;
    //跳过IP首部
    uint8_t *icmp = pkt + ipHeaderLen;

    char ipAddr[16];
    double rtt = 0;
    //传输中超时(IP头中的TTL变为0，icmp类型字段值为11，代码字段为0)
    if (ICMP_TIME_OUT_TYPE == *icmp && ICMP_TIME_OUT_INTRANS_CODE == *(icmp + 1)) {
      ssize_t timeOutIpHeaderLen = ((*(icmp + 8)) & 0x0f) << 2;
      icmp = icmp + 8 +
             timeOutIpHeaderLen;  //这里要跳过ICMP超时报文的头部和源IP数据报的首部，才能获取源ICMP的前8个字节的起始位置
      if (!checkSelfPkt(icmp)) return;
      rtt = getRtt();
      ifBreak = true;
    } else if (ICMP_ECHO_TYPE_RESP == *icmp) {  //正常的icmp echo应答包
      if (!checkSelfPkt(icmp)) return;
      rtt = getRtt(icmp + 8);
      done = true;
    }
    if (!(done || ifBreak)) return;
    if (memcmp(&recvAddr.sin_addr, &lastAddr.sin_addr, sinAddrSize) != 0) {
      cout << " " << inet_ntop(AF_INET, &recvAddr.sin_addr, ipAddr, 16) << flush;
    }
    lastAddr = recvAddr;
    printf(" (%.3fms)", rtt);
  }

  void recvIcmpEchoResp(int recvFd, bool &done) {
    int64_t preTime = time(NULL);
    struct timeval timeout;
    bzero(&timeout, sizeof(timeout));
    timeout.tv_sec = 1;
    setsockopt(recvFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));  //设置接收超时时间为1秒

    uint8_t pkt[IP_PROTO_MAX_SIZE];
    socklen_t len = sizeof(recvAddr);
    bool ifBreak = false;
    while (true) {
      ssize_t n = recvfrom(recvFd, pkt, IP_PROTO_MAX_SIZE, 0, (struct sockaddr *)&recvAddr, (socklen_t *)&len);
      gettimeofday(&recvTime, NULL);
      dealResp(recvFd, pkt, n, ifBreak, done);
      if (ifBreak || done) {
        break;
      }
      int64_t curTime = time(NULL);
      if (curTime - preTime >= 1) {
        cout << " *" << flush;  //超时
        break;
      }
    }
  }

  void run(char *host) {
    pid = getpid() & 0xffff;
    cout << "traceroute to " << host << "(" << inet_ntoa(addr.sin_addr) << "), 30 hops max" << endl;
    int sendFd = 0;
    int recvFd = 0;

    sendFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sendFd < 0) {
      perror("call socket failed!");
      return;
    }
    recvFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (recvFd < 0) {
      perror("call socket failed!");
      return;
    }
    bool done = false;
    seq = 0;
    for (int ttl = 1; ttl <= 30; ++ttl) {
      printf("%2d ", ttl);
      fflush(stdout);
      for (int i = 0; i < 3; ++i) {
        sendIcmpEchoReq(sendFd, ++seq, ttl);
        recvIcmpEchoResp(recvFd, done);
      }
      cout << endl;
      if (done) {
        break;
      }
    }
  }

 private:
  int16_t pid{0};
  int16_t seq{0};
  size_t sinAddrSize{0};
  struct timeval sendTime;
  struct timeval recvTime;
  struct sockaddr_in addr;
  struct sockaddr_in recvAddr;
  struct sockaddr_in lastAddr;
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "param invalid!" << endl;
    cout << "Usage: mytraceroute www.baidu.com" << endl;
    return -1;
  }
  TraceRoute route;
  if (!route.init(argv[1])) {
    cout << "mytraceroute: unknown host " << argv[1] << endl;
    return -1;
  }
  route.run(argv[1]);
  return 0;
}