#pragma once

#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <list>
#include <string>

namespace EchoServer {
enum DecodeStatus {
  HEAD = 1,    // 解析协议头（协议体长度）
  BODY = 2,    // 解析协议体
  FINISH = 3,  // 完成解析
};

class Packet {
 public:
  Packet() = default;
  ~Packet() {
    if (data_) delete[] data_;
    len_ = 0;
  }
  void Alloc(size_t size) {
    if (data_) delete[] data_;
    len_ = size;
    data_ = new uint8_t[len_];
  }
  uint8_t *Data() { return data_; }
  ssize_t Len() { return len_; }

 public:
  uint8_t *data_{nullptr};  // 二进制缓冲区
  ssize_t len_{0};          // 缓冲区的长度
};

class Codec {
 public:
  ~Codec() {
    if (msg_) delete msg_;
  }
  void EnCode(const std::string &msg, Packet &pkt) {
    pkt.Alloc(msg.length() + 4);
    *(uint32_t *)pkt.Data() = htonl(msg.length());  // 协议体长度转换成网络字节序
    memmove(pkt.Data() + 4, msg.data(), msg.length());
  }
  void DeCode(uint8_t *data, size_t len) {
    uint32_t decodeLen = 0;  // 本次解析的字节长度
    reserved_.append((const char *)data, len);
    uint32_t curLen = reserved_.size();  // 还有多少字节需要解析
    data = (uint8_t *)reserved_.data();
    if (nullptr == msg_) msg_ = new std::string("");
    while (curLen > 0) {  // 只要还有未解析的网络字节流，就持续解析
      bool decodeBreak = false;
      if (HEAD == decode_status_) {  // 解析协议头
        decodeHead(&data, curLen, decodeLen, decodeBreak);
        if (decodeBreak) break;
      }
      if (BODY == decode_status_) {  // 解析完协议头，解析协议体
        decodeBody(&data, curLen, decodeLen, decodeBreak);
        if (decodeBreak) break;
      }
    }
    if (decodeLen > 0) {  // 删除本次解析完的数据
      reserved_.erase(0, decodeLen);
    }
    if (reserved_.size() <= 0) {  // 及时释放空间
      reserved_.reserve(0);
    }
  }
  bool GetMessage(std::string &msg) {
    if (nullptr == msg_) return false;
    if (decode_status_ != FINISH) return false;
    msg = *msg_;
    delete msg_;
    msg_ = nullptr;
    return true;
  }

 private:
  bool decodeHead(uint8_t **data, uint32_t &curLen, uint32_t &decodeLen, bool &decodeBreak) {
    if (curLen < 4) {  // head固定4个字节
      decodeBreak = true;
      return true;
    }
    body_len_ = ntohl(*(uint32_t *)(*data));
    curLen -= 4;
    (*data) += 4;
    decodeLen += 4;
    decode_status_ = BODY;
    return true;
  }
  bool decodeBody(uint8_t **data, uint32_t &curLen, uint32_t &decodeLen, bool &decodeBreak) {
    if (curLen < body_len_) {
      decodeBreak = true;
      return true;
    }
    msg_->append((const char *)*data, body_len_);
    curLen -= body_len_;
    (*data) += body_len_;
    decodeLen += body_len_;
    decode_status_ = FINISH;
    body_len_ = 0;
    return true;
  }

 private:
  DecodeStatus decode_status_{HEAD};  // 当前解析状态
  std::string reserved_;              // 未解析的网络字节流
  uint32_t body_len_{0};              // 当前消息的协议体长度
  std::string *msg_{nullptr};         // 解析的消息
};
}  // namespace EchoServer