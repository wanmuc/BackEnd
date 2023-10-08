#include <arpa/inet.h>
#include <net/if.h>  //for ifreq
#include <netpacket/packet.h>  //for sockaddr_ll
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "param invalid!" << endl;
    cout << "Usage: myarp 10.104.64.1" << endl;
    return -1;
  }
  int packetSock = socket(AF_PACKET, SOCK_RAW, htons(0x0806));
  if (packetSock < 0) {
    perror("call packet sock failed!");
    return -1;
  }
  struct ifreq ifReq;  //网络接口请求
  struct sockaddr_ll llAddr;  //设备无关的物理地址
  memset(&ifReq, 0x0, sizeof(ifReq));  //初始化网络接口请求
  memset(&llAddr, 0x0, sizeof(llAddr));  //初始化物理地址
  memcpy(ifReq.ifr_name, "eth0", 4);  //设置要请求的网络接口名称
  if (ioctl(packetSock, SIOCGIFINDEX, &ifReq) != 0) {  //获取eth0接口内部index
    perror("call ioctl failed!");
    return -1;
  }
  llAddr.sll_ifindex = ifReq.ifr_ifindex;
  llAddr.sll_protocol = htons(0x0806);
  llAddr.sll_family = AF_PACKET;
  if (bind(packetSock, (struct sockaddr *)&llAddr, sizeof(llAddr)) < 0) {
    perror("call bind failed!");
    return -1;
  }
  if (ioctl(packetSock, SIOCGIFADDR, &ifReq) != 0) {  //获取eth0的ip地址
    perror("call ioctl failed!");
    return -1;
  }

  struct in_addr srcAddr, dstAddr;
  srcAddr = ((struct sockaddr_in *)&(ifReq.ifr_addr))->sin_addr;
  inet_pton(AF_INET, argv[1], &dstAddr);
  if (ioctl(packetSock, SIOCGIFHWADDR, &ifReq) != 0) {  //获取eth0的硬件地址
    perror("call ioctl failed!");
    return -1;
  }

  const size_t ethAddrLen = 6;  //以太网地址长度为6个字节
  const size_t arpPktLen = 42;  // arp请求包大小为42个字节(14 + 28)
  uint8_t arpPkt[arpPktLen] = {0};  // arp请求包字节流缓冲区
  char srcMacAddr[ethAddrLen] = {0};
  memcpy(srcMacAddr, ifReq.ifr_hwaddr.sa_data, ethAddrLen);

  uint8_t *pkt = arpPkt;
  //设置以太网首部
  memset(pkt, 0xff, ethAddrLen);  //设置以太网目的地址为广播地址
  pkt += ethAddrLen;
  memcpy(pkt, srcMacAddr, ethAddrLen);  //设置以太网源地址为eth0的mac地址
  pkt += ethAddrLen;
  *(uint16_t *)pkt = htons(0x0806);  //设置以太网帧类型为ARP
  pkt += 2;

  //设置arp请求
  *(uint16_t *)pkt = htons(0x0001);  //设置硬件类型为以太网
  pkt += 2;
  *(uint16_t *)pkt = htons(0x0800);  //设置协议类型为ip
  pkt += 2;
  *pkt = ethAddrLen;  //设置以太网地址长度
  ++pkt;
  *pkt = sizeof(struct in_addr);  //设置ip地址长度
  ++pkt;
  *(uint16_t *)pkt = htons(0x0001);  //设置为arp请求
  pkt += 2;
  memcpy(pkt, srcMacAddr, ethAddrLen);  //设置发送端以太网mac地址
  pkt += ethAddrLen;
  *(uint32_t *)pkt = *(uint32_t *)&srcAddr;  //设置发送端ip地址
  pkt += 4;
  memset(pkt, 0xff, ethAddrLen);  //设置目的以太网地址为广播地址
  pkt += ethAddrLen;
  *(uint32_t *)pkt = *(uint32_t *)&dstAddr;  //设置目的ip地址

  ssize_t ret = send(packetSock, &arpPkt, arpPktLen, 0);  //发送arp request
  if (ret != arpPktLen) {
    perror("send arp request failed!");
    return -1;
  }
  ret = recv(packetSock, &arpPkt, arpPktLen, 0);  //接收arp reply
  if (ret != arpPktLen) {
    perror("recv arp reply failed!");
    return -1;
  }

  pkt = arpPkt;
  pkt += 14;  //跳过以太网首部
  pkt += 14;  //跳到发送端IP地址的首字节
  cout << inet_ntoa(*(struct in_addr *)pkt) << " is at ";  //输出ip地址
  pkt -= 6;  //跳回发送端以太网地址字段的首字节。
  for (int i = 0; i < ethAddrLen; ++i) {  //输出以太网地址
    if (i == ethAddrLen - 1) {
      printf("%02x\n", pkt[i]);
    } else {
      printf("%02x:", pkt[i]);
    }
  }
  return 0;
}