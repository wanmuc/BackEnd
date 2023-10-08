#pragma once

#include "../common/statuscode.hpp"
#include "packet.hpp"

namespace Protocol {
//协议中使用的常量
constexpr uint8_t PROTO_MAGIC = 1;                //协议魔数
constexpr uint8_t PROTO_VERSION = 1;              //协议版本号
constexpr uint32_t PROTO_HEAD_LEN = 8;            //固定8个字节的头部
constexpr uint8_t PROTO_FLAG_IS_JSON = 0x1;       // body是否为json
constexpr uint8_t PROTO_FLAG_IS_ONEWAY = 0x2;     //是否为Oneway消息
constexpr uint8_t PROTO_FLAG_IS_FAST_RESP = 0x4;  //是否为FastResp消息
constexpr uint8_t PROTO_MAGIC_AND_VERSION = (PROTO_MAGIC << 4) | PROTO_VERSION;

// 协议头
typedef struct Head {
  uint8_t magic_and_version_{PROTO_MAGIC_AND_VERSION};  //协议魔数和版本号
  uint8_t flag_{0};                                     //协议的标志位
  uint16_t context_len_{0};                             //消息上下文序列化后的长度（压缩过的）
  uint32_t body_len_{0};                                //消息体序列化后的长度（压缩过的）
} Head;

// 协议消息
typedef struct MySvrMessage {
  void CopyFrom(const MySvrMessage &message) {
    head_ = message.head_;
    context_.CopyFrom(message.context_);
    body_.CopyFrom(message.body_);
  }
  bool IsFastResp() { return head_.flag_ & PROTO_FLAG_IS_FAST_RESP; }
  void EnableFastResp() { head_.flag_ |= PROTO_FLAG_IS_FAST_RESP; }
  bool IsOneway() { return head_.flag_ & PROTO_FLAG_IS_ONEWAY; }
  void EnableOneway() { head_.flag_ |= PROTO_FLAG_IS_ONEWAY; }
  bool BodyIsJson() { return head_.flag_ & PROTO_FLAG_IS_JSON; }
  void BodyEnableJson() { head_.flag_ |= PROTO_FLAG_IS_JSON; }
  int32_t StatusCode() { return context_.status_code(); }
  std::string Message() { return STATUS_CODE.Message(context_.status_code()); }

  Head head_;                     //消息头
  MySvr::Base::Context context_;  //消息上下文
  Packet body_;  //消息体（字节流），需要根据context_中的service_name和rpc_name去做反序列化成具体的请求对象
} MySvrMessage;
}  // namespace Protocol