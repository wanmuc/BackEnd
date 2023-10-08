#include <iostream>
#include <string>

#include "../../common/cmdline.h"
#include "../../common/robustio.hpp"
#include "../../core/distributedtrace.hpp"
#include "../../core/routeinfo.hpp"
#include "../../protocol/mixedcodec.hpp"

#define GREEN_BEGIN "\033[32m"
#define RED_BEGIN "\033[31m"
#define COLOR_END "\033[0m"

using namespace std;
using namespace Core;

string serviceName;
string rpcName;
string jsonStr;
bool byAccessService;
bool printTrace;
bool printContext;
bool oneway;
bool fastResp;

void usage() {
  cout << "myrpct -service_name Echo -rpc_name EchoMySelf -json "
       << "'{\"message\" : \"hello\"}' [-a] [-t] [-c]" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help     print usage" << endl;
  cout << "    -service_name service name" << endl;
  cout << "    -rpc_name     rpc name" << endl;
  cout << "    -json         json message" << endl;
  cout << "    -a            by access service" << endl;
  cout << "    -t            print trace info" << endl;
  cout << "    -c            print context info" << endl;
  cout << "    -o            is oneway message mode" << endl;
  cout << "    -f            is fast response message mode" << endl;
  cout << endl;
}

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
  int ret = connect(sockFd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret) {
    error = strerror(errno);
    cout << RED_BEGIN << "connect failed. " << error << COLOR_END << endl;
    return -1;
  }
  return sockFd;
}

Core::Route getRoute() {
  Core::Route route;
  Core::TimeOut timeOut;
  Core::RouteInfo routeInfo;
  if (byAccessService) {
    if (not routeInfo.GetRoute("access", route, timeOut)) {
      cout << RED_BEGIN << "can't get service_name[" << serviceName << "] route info" << COLOR_END << endl;
      exit(-1);
    }
  } else {
    if (not routeInfo.GetRoute(serviceName, route, timeOut)) {
      cout << RED_BEGIN << "can't get service_name[" << serviceName << "] route info" << COLOR_END << endl;
      exit(-1);
    }
  }
  return route;
}

void execTest() {
  Core::Route route = getRoute();
  int sockFd = createSockAndConnect(route);
  if (sockFd < 0) {
    exit(-1);
  }
  Protocol::Packet pkt;
  Protocol::MySvrCodec codec;
  Protocol::MySvrMessage reqMessage;
  // body使用json格式
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
    exit(-1);
  }
  if (oneway) {  // oneway消息发送成功即可，然后直接返回
    cout << GREEN_BEGIN << "oneway message send success" << COLOR_END << endl;
    return;
  }
  void* message;
  while (true) {
    if (robustIo.Read(codec.Data(), codec.Len()) != (ssize_t)codec.Len()) {
      cout << RED_BEGIN << "recv response failed." << COLOR_END << endl;
      exit(-1);
    }
    if (not codec.Decode(codec.Len())) {
      cout << RED_BEGIN << "decode response failed." << COLOR_END << endl;
      exit(-1);
    }
    message = codec.GetMessage();
    if (message) {
      break;
    }
  }
  Protocol::MySvrMessage respMessage;
  respMessage.CopyFrom(*(Protocol::MySvrMessage*)message);
  string contextJsonStr;
  Common::Convert::Pb2JsonStr(respMessage.context_, contextJsonStr, true);
  if (printContext) {
    cout << GREEN_BEGIN << "context[" << contextJsonStr << "]" << COLOR_END << endl;
  }
  if (printTrace) {
    int stackId = respMessage.context_.current_stack_id();
    DistributedTrace::PrintTraceInfo(respMessage.context_, stackId, 0, false, false);
  }
  string respJsonStr((char*)respMessage.body_.DataRaw(), respMessage.body_.UseLen());
  if (respMessage.IsFastResp()) {
    cout << GREEN_BEGIN << "is fast response, response is empty" << COLOR_END << endl;
  } else {
    cout << GREEN_BEGIN << "response[" << respJsonStr << "]" << COLOR_END << endl;
  }
  delete (Protocol::MySvrMessage*)message;
}

int main(int argc, char* argv[]) {
  Common::CmdLine::StrOptRequired(&serviceName, "service_name");
  Common::CmdLine::StrOptRequired(&rpcName, "rpc_name");
  Common::CmdLine::StrOptRequired(&jsonStr, "json");
  Common::CmdLine::BoolOpt(&byAccessService, "a");
  Common::CmdLine::BoolOpt(&printTrace, "t");
  Common::CmdLine::BoolOpt(&printContext, "c");
  Common::CmdLine::BoolOpt(&oneway, "o");
  Common::CmdLine::BoolOpt(&fastResp, "f");
  Common::CmdLine::SetUsage(usage);
  Common::CmdLine::Parse(argc, argv);
  execTest();
  return 0;
}