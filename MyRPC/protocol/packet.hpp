#pragma once

#include "base.pb.h"

namespace Protocol {
// 二进制包
class Packet {
 public:
  ~Packet() {
    if (data_) free(data_);
  }
  void Alloc(size_t len) {
    if (data_) free(data_);
    data_ = (uint8_t *)malloc(len);
    len_ = len;
    use_len_ = 0;
    parse_len_ = 0;
  }
  void ReAlloc(size_t len) {
    if (len < len_) {
      return;
    }
    data_ = (uint8_t *)realloc(data_, len);
    len_ = len;
  }
  void CopyFrom(const Packet &pkt) {
    if (data_) free(data_);
    data_ = (uint8_t *)malloc(pkt.len_);
    memmove(data_, pkt.data_, pkt.len_);
    len_ = pkt.len_;
    use_len_ = pkt.use_len_;
    parse_len_ = pkt.parse_len_;
  }
  uint8_t *Data() { return data_ + use_len_; }             //缓冲区可以写入的开始地址
  uint8_t *DataRaw() { return data_; }                     //原始缓冲区的开始地址
  uint8_t *DataParse() { return data_ + parse_len_; }      //需要解析的开始地址
  size_t NeedParseLen() { return use_len_ - parse_len_; }  //还需要解析的长度
  size_t Len() { return len_ - use_len_; }                 //缓存区中还可以写入的数据长度
  size_t UseLen() { return use_len_; }                     //缓冲区已经使用的容量
  void UpdateUseLen(size_t add_len) { use_len_ += add_len; }
  void UpdateParseLen(size_t add_len) { parse_len_ += add_len; }

 public:
  uint8_t *data_{nullptr};  // 二进制缓冲区
  size_t len_{0};           // 缓冲区的长度
  size_t use_len_{0};       // 缓冲区使用长度
  size_t parse_len_{0};     // 完成解析的长度
};
}  // namespace Protocol