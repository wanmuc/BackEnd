#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "../common/percentile.hpp"
#include "../common/singleton.hpp"
#include "coroutineio.hpp"
#include "coroutinelocal.hpp"

#define CONN_MANAGER Common::Singleton<Core::ConnManager>::Instance()

extern Core::CoroutineLocal<Core::TimeOut> RpcTimeOut;

namespace Core {
typedef struct Conn {
  int fd_{-1};                // 连接的fd
  bool finish_auth_{false};   // 是否完成了认证，需要做认证的协议，本字段才启用
  int64_t last_used_time_;    // 最近一次使用时间，单位秒
  std::string service_name_;  // 关联的服务
  TimeOut time_out_;          // 超时配置
} Conn;

class ConnManager {
 public:
  ~ConnManager() {
    for (auto &connList : conn_pools_) {
      for (Conn *conn : connList.second) {
        deleteConn(conn);
      }
    }
  }
  void SetMaxIdleTime(int64_t max_idle_time) { max_idle_time_ = max_idle_time; }
  Conn *Get(std::string serviceName) {  // 返回的都是完成connect的连接
    Conn *conn = nullptr;
    Common::Defer defer([this, &conn, serviceName]() {
      if (conn) {
        conn_stats_[serviceName] = conn_stats_[serviceName] + 1;
      }
    });
    auto iter = conn_pools_.find(serviceName);
    if (iter == conn_pools_.end()) {
      conn_pools_[serviceName] = std::list<Conn *>();
      conn_stats_[serviceName] = 0;
      conn = newConn(serviceName);
      return conn;
    }
    int count = 0;
    while (not iter->second.empty()) {
      conn = iter->second.front();
      iter->second.pop_front();
      // 长时间没请求时，突然来有请求时，需要处理过期的连接
      if (conn->last_used_time_ + max_idle_time_ < time(nullptr)) {
        count++;
        if (count <= 25) {  // 连接过期了，需要释放，每次最多释放25个连接，避免Get函数耗时过多
          deleteConn(conn);
          conn = nullptr;  // 这里需要设置为null，否则可能返回一个已经被释放的Conn
          continue;
        } else {
          iter->second.push_front(conn);  // 把conn再塞回列表
          conn = nullptr;
          break;
        }
      }
      break;
    }
    if (nullptr == conn) {  // 无法复用存量的连接，则尝试创建新的连接
      conn = newConn(serviceName);
      return conn;
    }
    if (connIsValid(conn)) {  // 连接还是可用的，则直接返回
      return conn;
    }
    // 执行到这里conn是不可用，则需要删除连接，再尝试创建新的连接
    deleteConn(conn);
    conn = newConn(serviceName);
    return conn;
  }
  void Put(Conn *conn) {  // 归还一个连接
    static Common::Percentile pct;
    assert(conn != nullptr);
    std::string serviceName = conn->service_name_;
    Common::Defer defer([this, serviceName]() {
      conn_stats_[serviceName] = conn_stats_[serviceName] - 1;  // 连接使用数减1
    });
    conn->last_used_time_ = time(nullptr);
    auto iter = conn_pools_.find(serviceName);
    assert(iter != conn_pools_.end());
    iter->second.push_back(conn);
    pct.Stat(serviceName, conn_stats_[serviceName]);
    double pctValue;
    if (not pct.GetPercentile(serviceName, 0.99, pctValue)) {
      return;
    }
    size_t remainCnt = (size_t)pctValue;
    while (iter->second.size() > 0 && iter->second.size() > remainCnt) {  // 释放多余的连接
      conn = iter->second.front();
      iter->second.pop_front();
      deleteConn(conn);
    }
  }
  void Release(Conn *conn) {
    std::string serviceName = conn->service_name_;
    assert(conn_stats_.find(serviceName) != conn_stats_.end());
    conn_stats_[serviceName] = conn_stats_[serviceName] - 1;  // 连接使用数减1
    deleteConn(conn);
  }

 private:
  bool connIsValid(Conn *conn) {
    RpcTimeOut.Set(conn->time_out_);  // 这里强制设置一下超时配置，避免协程进入之前没设置
    Common::Defer defer([conn]() { RpcTimeOut.Set(conn->time_out_); });  // 恢复超时配置
    RpcTimeOut.Get().read_time_out_ms_ = 1;  // 付出1毫秒的代价来检查连接是否已经被对端关闭
    char data;
    ssize_t ret = CoRead(conn->fd_, &data, 1);
    if (0 == ret) {  // 对端已经关闭了连接
      return false;
    }
    // 连接可用时，read操作会超时且错误码为EAGAIN，从而判断连接当前是可用的
    if (-1 == ret && errno == EAGAIN) {
      return true;
    }
    return false;
  }
  void deleteConn(Conn *conn) {
    assert(0 == close(conn->fd_));
    delete conn;
  }
  Conn *newConn(std::string serviceName) {
    Route route;
    TimeOut timeOut;
    if (not ROUTE_INFO.GetRoute(serviceName, route, timeOut)) {
      return nullptr;
    }
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);  // 创建socket，并设置成非阻塞的
    if (fd < 0) {
      ERROR("socket call failed. %s", strerror(errno));
      return nullptr;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(int16_t(route.port_));
    addr.sin_addr.s_addr = inet_addr(route.ip_.c_str());
    RpcTimeOut.Set(timeOut);
    int ret = CoConnect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret) {
      ERROR("CoConnect call failed. %s", strerror(errno));
      assert(0 == close(fd));
      return nullptr;
    }
    Conn *conn = new Conn;
    conn->fd_ = fd;
    conn->last_used_time_ = time(nullptr);
    conn->service_name_ = serviceName;
    conn->time_out_ = timeOut;
    return conn;
  }

 private:
  int64_t max_idle_time_{300};                           // 连接最大空闲时间，单位秒，默认5分钟
  std::map<std::string, std::list<Conn *>> conn_pools_;  // 连接池
  std::map<std::string, int32_t> conn_stats_;            // 连接使用统计
};
}  // namespace Core