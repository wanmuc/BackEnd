#include "../common/config.hpp"
#include "unittestcore.h"

TEST_CASE(Config_All) {
  Common::Config config;
  ASSERT_TRUE(config.Load("./mysvr.conf"));
  int64_t port;
  config.GetIntValue("MySvr", "port", port, 0);
  ASSERT_EQ(port, 1688);
  std::string ip;
  config.GetStrValue("MySvr", "ip", ip, "");
  ASSERT_EQ(ip, "127.0.0.1");
  config.GetStrValue("MySvr", "ip2", ip, "127");
  ASSERT_EQ(ip, "127");
  std::string serviceName;
  config.GetStrValue("MySvr2", "service_name", serviceName, "127");
  ASSERT_EQ(serviceName, "echo_server");
  std::string rpcName;
  config.GetStrValue("MySvr2", "rpc_name", rpcName, "127");
  ASSERT_EQ(rpcName, "ping");
  config.GetStrValue("MySvr22", "rpc_name", rpcName, "127");
  ASSERT_EQ(rpcName, "127");
}