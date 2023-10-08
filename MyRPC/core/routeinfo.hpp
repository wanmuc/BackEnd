#pragma once

#include <map>
#include <string>
#include <vector>

#include "../common/config.hpp"
#include "../common/log.hpp"
#include "../common/singleton.hpp"

#define ROUTE_INFO Common::Singleton<Core::RouteInfo>::Instance()

namespace Core {
typedef struct Route {
  std::string ip_;
  int64_t port_;
} Route;

typedef struct TimeOut {
  int64_t read_time_out_ms_{1000};
  int64_t write_time_out_ms_{1000};
  int64_t connect_time_out_ms_{50};
} TimeOut;

class RouteInfo {
 public:
  void SetExpireTime(int64_t expire_time) { expire_time_ = expire_time; }
  bool GetRoute(std::string serviceName, Route &route, TimeOut &timeOut, int index = 0) {
    Common::Strings::ToLower(serviceName);
    int64_t currentTime = time(nullptr);
    auto update_time_iter = last_update_times_.find(serviceName);
    if (update_time_iter == last_update_times_.end() || update_time_iter->second + expire_time_ < currentTime) {
      last_update_times_[serviceName] = currentTime;
      updateRoute(serviceName);
    }
    auto iter = route_infos_.find(serviceName);
    if (iter == route_infos_.end()) {
      ERROR("get Route failed. serviceName[%s]", serviceName.c_str());
      return false;
    }
    ERROR("get Route success. serviceName[%s]", serviceName.c_str());
    if (0 == index) index = rand();
    route = iter->second[index % iter->second.size()];  // 返回的路由信息
    timeOut = time_outs_[serviceName];
    return true;
  }

 private:
  void updateRoute(std::string &serviceName) {
    std::string routeFile = "/home/backend/route/" + serviceName + "_client.conf";
    Common::Config config;
    if (not config.Load(routeFile)) {  // 加载路由文件失败
      ERROR("routeFile[%s] load failed.", routeFile.c_str());
      return;
    }
    config.Dump([](const std::string &section, const std::string &key, const std::string &value) {
      INFO("section[%s],keyValue[%s=%s]", section.c_str(), key.c_str(), value.c_str());
    });
    int64_t count = 0;
    config.GetIntValue("Svr", "count", count, 0);
    if (count <= 0) return;
    std::vector<Route> routeInfos;
    for (int64_t i = 1; i <= count; i++) {
      Route temp;
      std::string section = "Svr" + std::to_string(i);
      config.GetIntValue(section, "port", temp.port_, 0);
      config.GetStrValue(section, "ip", temp.ip_, "");
      routeInfos.push_back(temp);
    }
    if (routeInfos.size() <= 0) return;
    TimeOut timeOut;
    config.GetIntValue("Svr", "connectTimeOutMs", timeOut.connect_time_out_ms_, 50);
    config.GetIntValue("Svr", "readTimeOutMs", timeOut.read_time_out_ms_, 1000);
    config.GetIntValue("Svr", "writeTimeOutMs", timeOut.write_time_out_ms_, 1000);
    time_outs_[serviceName] = timeOut;
    route_infos_[serviceName] = routeInfos;
  }

 private:
  int64_t expire_time_{300};  // 过期时间，单位秒
  std::map<std::string, TimeOut> time_outs_;  // 超时配置
  std::map<std::string, int64_t> last_update_times_;  // 最后更新时间，单位秒
  std::map<std::string, std::vector<Route>> route_infos_;  // 各个模块的路由信息
};
}  // namespace Core