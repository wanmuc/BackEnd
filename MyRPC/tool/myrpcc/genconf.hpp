#pragma once
#include "genbase.hpp"
#include "protosimpleparser.hpp"

class GenConf : public GenBase {
 public:
  static bool Gen(ServiceInfo &serviceInfo) {
    if (not ExecCmd("mkdir -p ./conf")) return false;
    if (not genConf(serviceInfo)) return false;
    if (not genClientConf(serviceInfo)) return false;
    return true;
  }

 private:
  static bool genConf(ServiceInfo &serviceInfo) {
    std::string file = "./conf/" + serviceInfo.handler_file_prefix_ + ".conf";
    if (IsFileExist(file)) {
      return true;
    }
    string content = R"([MyRPC]
port = )" + serviceInfo.port_ +
                     R"(
listen_if = any
coroutine_count = 10240
process_count = 16)";
    GenFile(file, content);
    return true;
  }
  static bool genClientConf(ServiceInfo &serviceInfo) {
    std::string file = "./conf/" + serviceInfo.handler_file_prefix_ + "_client.conf";
    if (IsFileExist(file)) {
      return true;
    }
    string content = R"([Svr]
count = 1
connectTimeOutMs = 50
readTimeOutMs = 1000
writeTimeOutMs = 1000
[Svr1]
ip = 127.0.0.1
port = )" + serviceInfo.port_;
    GenFile(file, content);
    return true;
  }
};
