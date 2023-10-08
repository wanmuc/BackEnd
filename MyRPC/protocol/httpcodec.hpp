#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../common/log.hpp"
#include "../common/strings.hpp"
#include "codec.hpp"
#include "httpmessage.hpp"

namespace Protocol {
constexpr uint32_t FIRST_READ_LEN = 256;           //优先读取数据的大小
constexpr uint32_t MAX_FIRST_LINE_LEN = 8 * 1024;  // http第一行的最大长度
constexpr uint32_t MAX_HEADER_LEN = 8 * 1024;      // header最大长度
constexpr uint32_t MAX_BODY_LEN = 1024 * 1024;     // body最大长度
// 解码状态
enum HttpDecodeStatus {
  FIRST_LINE = 1,  // 第一行
  HEADERS = 2,     // 消息头
  BODY = 3,        // 消息体
  FINISH = 4,      // 完成了消息解析
};
// 协议编解码
class HttpCodec : public Codec {
 public:
  HttpCodec() { pkt_.Alloc(FIRST_READ_LEN); }
  ~HttpCodec() {
    if (message_) delete message_;
  }
  CodecType Type() { return HTTP; }
  void *GetMessage() {
    if (nullptr == message_ || decode_status_ != FINISH) return nullptr;
    HttpMessage *result = message_;
    message_ = nullptr;
    decode_status_ = FIRST_LINE;  //消息被取出之后解析状态设置为FIRST_LINE
    return result;
  }
  void SetLimit(uint32_t maxFirstLineLen, uint32_t maxHeaderLen, uint32_t maxBodyLen) {
    max_first_line_len_ = maxFirstLineLen;
    max_header_len_ = maxHeaderLen;
    max_body_len_ = maxBodyLen;
  }
  bool Encode(void *msg, Packet &pkt) {
    HttpMessage *message = (HttpMessage *)msg;
    std::string data;
    data.append(message->first_line_ + "\r\n");
    auto iter = message->headers_.begin();
    while (iter != message->headers_.end()) {
      data.append(iter->first + ": " + iter->second + "\r\n");
      iter++;
    }
    data.append("\r\n");
    data.append(message->body_);
    pkt.Alloc(data.length());
    memmove(pkt.Data(), data.c_str(), data.length());
    pkt.UpdateUseLen(data.length());
    return true;
  }
  bool Decode(size_t len) {
    pkt_.UpdateUseLen(len);
    uint32_t decodeLen = 0;
    uint32_t needDecodeLen = pkt_.NeedParseLen();
    uint8_t *data = pkt_.DataParse();
    if (nullptr == message_) message_ = new HttpMessage;
    while (needDecodeLen > 0) {  //只要还有未解析的网络字节流，就持续解析
      bool decodeBreak = false;
      if (FIRST_LINE == decode_status_) {  //解析第一行
        if (not decodeFirstLine(&data, needDecodeLen, decodeLen, decodeBreak)) {
          return false;
        }
        if (decodeBreak) break;
      }
      if (needDecodeLen > 0 && HEADERS == decode_status_) {  //解析完第一行，解析headers
        if (not decodeHeaders(&data, needDecodeLen, decodeLen, decodeBreak)) {
          return false;
        }
        if (decodeBreak) break;
      }
      if (needDecodeLen > 0 && BODY == decode_status_) {  //解析完headers，解析body
        if (not decodeBody(&data, needDecodeLen, decodeLen, decodeBreak)) {
          return false;
        }
        if (decodeBreak) break;
      }
    }
    if (decodeLen > 0) pkt_.UpdateParseLen(decodeLen);
    if (FINISH == decode_status_) {
      pkt_.Alloc(FIRST_READ_LEN);  // 解析完一个消息及时释放空间，并申请新的空间
    }
    return true;
  }

 private:
  bool decodeFirstLine(uint8_t **data, uint32_t &needDecodeLen, uint32_t &decodeLen, bool &decodeBreak) {
    uint8_t *temp = *data;
    bool completeFirstLine = false;
    uint32_t firstLineLen = 0;
    for (uint32_t i = 0; i < needDecodeLen - 1; i++) {
      if (temp[i] == '\r' && temp[i + 1] == '\n') {
        completeFirstLine = true;
        firstLineLen = i + 2;
        break;
      }
    }
    if (not completeFirstLine) {
      if (needDecodeLen > max_first_line_len_) {
        ERROR("first_line len[%d] is too long", needDecodeLen);
        return false;
      }
      pkt_.ReAlloc(pkt_.UseLen() * 2);  // 无法完成第一行的解析，则尝试扩大下次读取的数据量
      decodeBreak = true;
      return true;
    }
    if (firstLineLen > max_first_line_len_) {
      ERROR("first_line len[%d] is too long", firstLineLen);
      return false;
    }
    message_->first_line_ = std::string((char *)temp, firstLineLen - 2);
    //更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= firstLineLen;
    decodeLen += firstLineLen;
    (*data) += firstLineLen;
    decode_status_ = HEADERS;
    return true;
  }
  bool decodeHeaders(uint8_t **data, uint32_t &needDecodeLen, uint32_t &decodeLen, bool &decodeBreak) {
    uint8_t *temp = *data;
    if (needDecodeLen >= 2 && temp[0] == '\r' && temp[1] == '\n') {  //解析到空行
      needDecodeLen -= 2;
      decodeLen += 2;
      (*data) += 2;
      decode_status_ = BODY;
      return true;
    }
    bool isKey = true;
    uint32_t decodeHeadersLen = 0;
    bool getOneHeader = false;
    // 解析每个header的key，value对
    std::string key, value;
    for (uint32_t i = 0; i < needDecodeLen - 1; i++) {
      if (temp[i] == '\r' && temp[i + 1] == '\n') {  // 一个完整的key，value对
        Common::Strings::trim(key);
        Common::Strings::trim(value);
        if (key != "" && value != "") message_->headers_[key] = value;
        getOneHeader = true;
        decodeHeadersLen = i + 2;
        break;
      }
      if (isKey && temp[i] == ':') {  // 第一个':'才是分隔符
        isKey = false;
        continue;
      }
      if (isKey)
        key += temp[i];
      else
        value += temp[i];
    }
    if (not getOneHeader) {
      if (needDecodeLen > max_header_len_) {
        ERROR("header len[%d] is too long", needDecodeLen);
        return false;
      }
      decodeBreak = true;
      pkt_.ReAlloc(pkt_.UseLen() * 2);  // 无法完成headers的解析，则尝试扩大下次读取的数据量
      return true;
    }
    if (decodeHeadersLen > max_header_len_) {
      ERROR("header len[%d] is too long", decodeHeadersLen);
      return false;
    }
    needDecodeLen -= decodeHeadersLen;
    decodeLen += decodeHeadersLen;
    (*data) += decodeHeadersLen;
    return true;
  }
  bool decodeBody(uint8_t **data, uint32_t &needDecodeLen, uint32_t &decodeLen, bool &decodeBreak) {
    auto iter = message_->headers_.find("Content-Length");
    if (iter == message_->headers_.end()) {  // 只支持通过Content-Length来标识body的长度
      ERROR("not find Content-Length header");
      return false;
    }
    uint32_t bodyLen = (uint32_t)std::stoi(iter->second.c_str());
    if (bodyLen > max_body_len_) {
      ERROR("body len[%d] is too long", bodyLen);
      return false;
    }
    decodeBreak = true;  // 不管是否能完成解析都跳出循环
    if (needDecodeLen < bodyLen) {
      pkt_.ReAlloc(pkt_.UseLen() + (bodyLen - needDecodeLen));  // 无法完成解析，则尝试扩大下次读取的数据量
      return true;
    }
    uint8_t *temp = *data;
    message_->body_ = std::string((char *)temp, bodyLen);
    //更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= bodyLen;
    decodeLen += bodyLen;
    (*data) += bodyLen;
    decode_status_ = FINISH;  //解析状态流转，更新为完成消息解析
    return true;
  }

 private:
  HttpDecodeStatus decode_status_{FIRST_LINE};  // 当前解析状态
  HttpMessage *message_{nullptr};
  uint32_t max_first_line_len_{MAX_FIRST_LINE_LEN};
  uint32_t max_header_len_{MAX_HEADER_LEN};
  uint32_t max_body_len_{MAX_BODY_LEN};
};
}  // namespace Protocol