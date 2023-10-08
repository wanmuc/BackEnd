#pragma once

#include <arpa/inet.h>
#include <snappy.h>

#include <string>

#include "codec.hpp"
#include "mysvrmessage.hpp"

namespace Protocol {
constexpr uint32_t MY_SVR_MAX_CONTEXT_LEN = 64 * 1024;      // 消息上下文最大长度
constexpr uint32_t MY_SVR_MAX_BODY_LEN = 20 * 1024 * 1024;  // 消息体最大长度
// 解码状态
enum MySvrDecodeStatus {
  MY_SVR_HEAD = 1,     // 消息头
  MY_SVR_CONTEXT = 2,  // 消息上下文
  MY_SVR_BODY = 3,     // 消息体
  MY_SVR_FINISH = 4,   // 完成了消息解析
};
// 协议编解码
class MySvrCodec : public Codec {
 public:
  MySvrCodec() { pkt_.Alloc(PROTO_HEAD_LEN); }  // 缓冲区第一次只分配协议头大小的空间
  ~MySvrCodec() {
    if (message_) delete message_;
  }
  CodecType Type() { return MY_SVR; }
  void *GetMessage() {
    if (nullptr == message_) return nullptr;
    if (decode_status_ != MY_SVR_FINISH) return nullptr;
    MySvrMessage *result = message_;
    message_ = nullptr;
    decode_status_ = MY_SVR_HEAD;  //消息被取出之后解析状态设置为HEAD
    return (void *)result;
  }
  void SetLimit(uint32_t maxContextLen, uint32_t maxBodyLen) {
    max_context_len_ = maxContextLen;
    max_body_len_ = maxBodyLen;
  }
  bool Encode(void *msg, Packet &pkt) {
    MySvrMessage &message = *(MySvrMessage *)msg;
    std::string body((const char *)message.body_.DataRaw(), message.body_.UseLen());
    std::string context;
    std::string compressBody;
    std::string compressContext;
    if (not message.context_.SerializePartialToString(&context)) return false;
    snappy::Compress(body.data(), body.size(), &compressBody);
    snappy::Compress(context.data(), context.size(), &compressContext);
    message.head_.context_len_ = compressContext.size();  //设置消息上下文的长度
    message.head_.body_len_ = compressBody.size();        //设置消息体的长度
    size_t len = PROTO_HEAD_LEN + message.head_.context_len_ + message.head_.body_len_;  //计算包总长度
    pkt.Alloc(len);                                                                      //分配空间
    encodeHead(message, pkt);                                                            // 打包消息头
    pkt.UpdateUseLen(PROTO_HEAD_LEN);
    memmove(pkt.Data(), compressContext.data(), compressContext.size());  //打包消息上下文
    pkt.UpdateUseLen(compressContext.size());
    memmove(pkt.Data(), compressBody.data(), compressBody.size());  //打包消息体
    pkt.UpdateUseLen(compressBody.size());
    return true;
  }
  bool Decode(size_t len) {
    pkt_.UpdateUseLen(len);
    uint32_t decodeLen = 0;
    uint32_t needDecodeLen = pkt_.NeedParseLen();
    uint8_t *data = pkt_.DataParse();
    if (nullptr == message_) message_ = new MySvrMessage;
    while (needDecodeLen > 0) {  //只要还有未解析的网络字节流，就持续解析
      bool decodeBreak = false;
      if (MY_SVR_HEAD == decode_status_) {  //解析消息头
        if (not decodeHead(&data, needDecodeLen, decodeLen, decodeBreak)) {
          return false;
        }
        if (decodeBreak) break;
      }
      if (MY_SVR_CONTEXT == decode_status_) {  //解析完消息头，解析消息上下文
        if (not decodeContext(&data, needDecodeLen, decodeLen, decodeBreak)) {
          return false;
        }
        if (decodeBreak) break;
      }
      if (MY_SVR_BODY == decode_status_) {  //解析完消息上下文，解析消息体
        if (not decodeBody(&data, needDecodeLen, decodeLen, decodeBreak)) {
          return false;
        }
        if (decodeBreak) break;
      }
    }
    if (decodeLen > 0) pkt_.UpdateParseLen(decodeLen);
    if (MY_SVR_FINISH == decode_status_) {
      pkt_.Alloc(PROTO_HEAD_LEN);  // 解析完一个消息及时释放空间，并申请协议头部需要的空间
    }
    return true;
  }

 private:
  void encodeHead(MySvrMessage &message, Packet &pkt) {
    uint8_t *data = pkt.Data();
    *data = message.head_.magic_and_version_;  //设置协议魔数和版本号
    ++data;
    *data = message.head_.flag_;  //设置协议flag
    ++data;
    *(uint16_t *)data = htons(message.head_.context_len_);  //设置消息上下文长度
    data += 2;
    *(uint32_t *)data = htonl(message.head_.body_len_);  //设置消息体长度
  }
  bool decodeHead(uint8_t **data, uint32_t &needDecodeLen, uint32_t &decodeLen, bool &decodeBreak) {
    if (needDecodeLen < PROTO_HEAD_LEN) {
      decodeBreak = true;
      return true;
    }
    uint8_t *curData = *data;
    message_->head_.magic_and_version_ = *curData;
    if (message_->head_.magic_and_version_ != PROTO_MAGIC_AND_VERSION) return false;  //魔数和版本号不一致，返回解析失败
    curData++;
    message_->head_.flag_ = *curData;  //解析标志位
    curData++;
    message_->head_.context_len_ = ntohs(*(uint16_t *)curData);  //解析消息上下文长度
    curData += 2;
    message_->head_.body_len_ = ntohl(*(uint32_t *)curData);  //解析消息体长度
    if (message_->head_.context_len_ > max_context_len_) return false;
    if (message_->head_.body_len_ > max_body_len_) return false;
    //更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= PROTO_HEAD_LEN;
    decodeLen += PROTO_HEAD_LEN;
    (*data) += PROTO_HEAD_LEN;
    decode_status_ = MY_SVR_CONTEXT;
    // 重新分配内存空间，这样解析一个消息最多就分配两次内存
    pkt_.ReAlloc(PROTO_HEAD_LEN + message_->head_.context_len_ + message_->head_.body_len_);
    return true;
  }
  bool decodeContext(uint8_t **data, uint32_t &needDecodeLen, uint32_t &decodeLen, bool &decodeBreak) {
    uint32_t contextLen = message_->head_.context_len_;
    if (needDecodeLen < contextLen) {
      decodeBreak = true;
      return true;
    }
    std::string context;
    std::string compressContext((const char *)*data, (size_t)contextLen);
    snappy::Uncompress(compressContext.data(), compressContext.size(), &context);
    if (not message_->context_.ParseFromString(context)) {
      return false;
    }
    //更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= contextLen;
    decodeLen += contextLen;
    (*data) += contextLen;
    decode_status_ = MY_SVR_BODY;
    return true;
  }
  bool decodeBody(uint8_t **data, uint32_t &needDecodeLen, uint32_t &decodeLen, bool &decodeBreak) {
    decodeBreak = true;  // 不管是否能完成解析都跳出循环
    uint32_t bodyLen = message_->head_.body_len_;
    if (needDecodeLen < bodyLen) {
      return true;
    }
    std::string body;
    std::string compressBody((const char *)*data, (size_t)bodyLen);
    snappy::Uncompress(compressBody.data(), compressBody.size(), &body);
    message_->body_.Alloc(body.size());
    memmove(message_->body_.Data(), body.data(), body.size());
    message_->body_.UpdateUseLen(body.size());
    //更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= bodyLen;
    decodeLen += bodyLen;
    (*data) += bodyLen;
    decode_status_ = MY_SVR_FINISH;  //解析状态流转，更新为完成消息解析
    return true;
  }

 private:
  MySvrDecodeStatus decode_status_{MY_SVR_HEAD};  // 当前解析状态
  MySvrMessage *message_{nullptr};                // 当前解析的消息对象
  uint32_t max_context_len_{MY_SVR_MAX_CONTEXT_LEN};
  uint32_t max_body_len_{MY_SVR_MAX_BODY_LEN};
};
}  // namespace Protocol