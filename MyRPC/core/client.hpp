#pragma once

#include <functional>

#include "../common/statuscode.hpp"
#include "../protocol/codec.hpp"
#include "connmanager.hpp"
#include "coroutineio.hpp"
#include "coroutinelocal.hpp"

extern Core::CoroutineLocal<Core::TimeOut> RpcTimeOut;  // rpc调用超时配置

namespace Core {
class Client {
 protected:
  bool PushRetry(std::string serviceName, Protocol::Codec &codec, void *pushMessage,
                 std::function<void(int, std::string)> sockErrorDeal) {
    int statusCode = 0;
    std::string error = "";
    for (int i = 0; i < 3; i++) {
      Conn *conn = getConn(serviceName);
      if (nullptr == conn) {
        WARN("get conn failed. serviceName[%s]", serviceName.c_str());
        statusCode = CONNECTION_FAILED;
        error = "get conn failed";
        continue;
      }
      RpcTimeOut.Set(conn->time_out_);
      if (writeMessage(codec, pushMessage, conn->fd_, statusCode, error)) {
        CONN_MANAGER.Put(conn);
        return true;
      }
      CONN_MANAGER.Release(conn);
    }
    sockErrorDeal(statusCode, error);
    return false;
  }
  bool CallRetry(std::string serviceName, Protocol::Codec &codec, void *reqMessage, void **respMessage,
                 std::function<bool(Conn *, std::string &)> connCallBack,
                 std::function<void(int, std::string)> errorDeal) {
    int statusCode = 0;
    std::string error = "";
    for (int i = 0; i < 3; i++) {
      Conn *conn = getConn(serviceName);
      if (nullptr == conn) {
        WARN("get conn failed. serviceName[%s]", serviceName.c_str());
        statusCode = CONNECTION_FAILED;
        error = "get conn failed";
        continue;
      }
      std::string connCallBackError;
      if (connCallBack && not connCallBack(conn, connCallBackError)) {
        WARN("connCallBack failed. serviceName[%s]", serviceName.c_str());
        CONN_MANAGER.Release(conn);
        statusCode = CONNECTION_FAILED;
        error = "conn call back failed. " + connCallBackError;
        continue;
      }
      RpcTimeOut.Set(conn->time_out_);
      if (not writeMessage(codec, reqMessage, conn->fd_, statusCode, error)) {
        CONN_MANAGER.Release(conn);
        continue;
      }
      if (not readMessage(codec, respMessage, conn->fd_, statusCode, error)) {
        CONN_MANAGER.Release(conn);
        continue;
      }
      CONN_MANAGER.Put(conn);
      return true;
    }
    errorDeal(statusCode, error);
    return false;
  }
  bool Call(Conn *conn, Protocol::Codec &codec, void *reqMessage, void **respMessage,
            std::function<void(int, std::string)> errorDeal) {
    int statusCode = 0;
    std::string error = "";
    RpcTimeOut.Set(conn->time_out_);
    if (not writeMessage(codec, reqMessage, conn->fd_, statusCode, error)) {
      errorDeal(statusCode, error);
      return false;
    }
    if (not readMessage(codec, respMessage, conn->fd_, statusCode, error)) {
      errorDeal(statusCode, error);
      return false;
    }
    return true;
  }

 private:
  Conn *getConn(std::string serviceName) {
    for (int i = 0; i < 3; i++) {  // 重试3次，解决一些网络抖动导致的连接获取失败问题
      Conn *conn = CONN_MANAGER.Get(serviceName);
      if (conn) return conn;
      WARN("get conn failed. count=%d", i + 1);
    }
    return nullptr;
  }
  bool writeMessage(Protocol::Codec &codec, void *message, int fd, int &statusCode, std::string &error) {
    Protocol::Packet pkt;
    codec.Encode(message, pkt);
    ssize_t sendLen = 0;
    uint8_t *buf = pkt.DataRaw();
    ssize_t needSendLen = pkt.UseLen();
    while (sendLen != needSendLen) {
      ssize_t ret = CoWrite(fd, buf + sendLen, needSendLen - sendLen);
      if (ret < 0) {
        statusCode = WRITE_FAILED;
        error = std::string("CoWrite failed.") + strerror(errno);
        return false;
      }
      sendLen += ret;
    }
    return true;
  }
  bool readMessage(Protocol::Codec &codec, void **respMessage, int fd, int &statusCode, std::string &error) {
    while (true) {
      ssize_t ret = CoRead(fd, codec.Data(), codec.Len());
      if (0 == ret) {
        statusCode = READ_FAILED;
        error = "peer close connection";
        return false;
      }
      if (ret < 0) {
        statusCode = READ_FAILED;
        error = std::string("CoRead failed. ") + strerror(errno);
        return false;
      }
      if (not codec.Decode(ret)) {
        statusCode = PARSE_FAILED;
        error = "decode failed.";
        return false;
      }
      *respMessage = codec.GetMessage();
      if ((*respMessage)) {
        break;
      }
    }
    return true;
  }
};
}  // namespace Core