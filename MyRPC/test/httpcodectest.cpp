#include "../protocol/httpcodec.hpp"
#include "unittestcore.h"

void printHttpMessage(Protocol::HttpMessage* message) {
  std::cout << "first line[" << message->first_line_ << "]" << std::endl;
  for (auto item : message->headers_) {
    std::cout << "header[" << item.first << "=" << item.second << "]" << std::endl;
  }
  std::cout << "body[" << message->body_ << "]" << std::endl;
}

TEST_CASE(HttpCodec_Decode_EnCode_Request) {
  std::string rawReq =
      "GET /mix/76.html?name=youname&age=6666 HTTP/1.1\r\n"
      "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
      "Accept-Encoding: gzip, deflate, sdch\r\n"
      "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\n"
      "Content-Length: 8\r\n"
      "Host: www.fishbay.cn\r\n"
      "Upgrade-Insecure-Requests: 1\r\n"
      "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_5) AppleWebKit/537.36 (KHTML, like Gecko) "
      "Chrome/56.0.2924.87 Safari/537.36\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  for (size_t i = 0; i < 5; i++) {
    for (size_t j = 0; j < rawReq.length(); j++) {
      *codec.Data() = (uint8_t)rawReq[j];
      codec.Decode(1);
    }
    Protocol::HttpMessage* message = (Protocol::HttpMessage*)codec.GetMessage();
    bool result = message != nullptr;
    ASSERT_TRUE(result);
    printHttpMessage(message);
    Protocol::Packet pkt;
    codec.Encode(message, pkt);
    std::string temp((char*)pkt.DataRaw(), pkt.UseLen());
    ASSERT_EQ(rawReq, temp);
    delete message;
  }
}

TEST_CASE(HttpCodec_Decode_EnCode_Response) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 8\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  for (size_t i = 0; i < 5; i++) {
    for (size_t j = 0; j < rawResp.length(); j++) {
      *codec.Data() = (uint8_t)rawResp[j];
      codec.Decode(1);  // 逐个字节进行解析
    }
    Protocol::HttpMessage* message = (Protocol::HttpMessage*)codec.GetMessage();
    bool result = message != nullptr;
    ASSERT_TRUE(result);
    printHttpMessage(message);
    Protocol::Packet pkt;
    codec.Encode(message, pkt);
    std::string temp((char*)pkt.DataRaw(), pkt.UseLen());
    ASSERT_EQ(rawResp, temp);
    delete message;
  }
}

TEST_CASE(HttpCodec_Decode_Error_First_Line_Too_Long1) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 8\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  codec.SetLimit(10, 1024, 1024);
  ASSERT_GE(codec.Len(), rawResp.size());
  memcpy((void*)codec.Data(), (void*)rawResp.data(), rawResp.size());
  bool result = codec.Decode(rawResp.size());
  ASSERT_FALSE(result);
}

TEST_CASE(HttpCodec_Decode_Error_First_Line_Too_Long2) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 8\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  codec.SetLimit(10, 1024, 1024);
  ASSERT_GE(codec.Len(), 14);
  memcpy((void*)codec.Data(), (void*)rawResp.data(), 14);
  bool result = codec.Decode(14);
  ASSERT_FALSE(result);
}

TEST_CASE(HttpCodec_Decode_Error_Header_Too_Long1) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 8\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  codec.SetLimit(1024, 10, 1024);
  ASSERT_GE(codec.Len(), rawResp.size());
  memcpy((void*)codec.Data(), (void*)rawResp.data(), rawResp.size());
  bool result = codec.Decode(rawResp.size());
  ASSERT_FALSE(result);
}

TEST_CASE(HttpCodec_Decode_Error_Header_Too_Long2) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 8\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  codec.SetLimit(1024, 5, 1024);
  ASSERT_GE(codec.Len(), 33);
  memcpy((void*)codec.Data(), (void*)rawResp.data(), 33);
  bool result = codec.Decode(33);
  ASSERT_FALSE(result);
}

TEST_CASE(HttpCodec_Decode_Error_Body_Too_Long) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 8\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  codec.SetLimit(1024, 1024, 7);
  ASSERT_GE(codec.Len(), rawResp.size());
  memcpy((void*)codec.Data(), (void*)rawResp.data(), rawResp.size());
  bool result = codec.Decode(rawResp.size());
  ASSERT_FALSE(result);
}

TEST_CASE(HttpCodec_Decode_Error_Not_Content_Length) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::HttpCodec codec;
  codec.SetLimit(1024, 1024, 1024);
  ASSERT_GE(codec.Len(), rawResp.size());
  memcpy((void*)codec.Data(), (void*)rawResp.data(), rawResp.size());
  bool result = codec.Decode(rawResp.size());
  ASSERT_FALSE(result);
}
