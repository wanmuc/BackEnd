#include "../protocol/mixedcodec.hpp"
#include "unittestcore.h"

TEST_CASE(MixedCodec_GetCodecTtype_MYSVR) {
  Protocol::MixedCodec codec;
  ASSERT_EQ(codec.GetCodecType(), Protocol::UNKNOWN);
  *codec.Data() = Protocol::PROTO_MAGIC_AND_VERSION;
  codec.Decode(1);
  ASSERT_EQ(codec.GetCodecType(), Protocol::MY_SVR);
  ASSERT_NE(codec.Len(), 1);
}

TEST_CASE(MixedCodec_GetCodecTtype_HTTP) {
  Protocol::MixedCodec codec;
  ASSERT_EQ(codec.GetCodecType(), Protocol::UNKNOWN);
  *codec.Data() = 'h';
  codec.Decode(1);
  ASSERT_EQ(codec.GetCodecType(), Protocol::HTTP);
  ASSERT_NE(codec.Len(), 1);
}

TEST_CASE(MixedCodec_Decode_Encode_HTTP) {
  std::string rawResp =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 8\r\n"
      "Content-Type: text/plain;charset=UTF-8\r\n"
      "Date: Mon, 20 Feb 2017 09:13:59 GMT\r\n"
      "Server: nginx\r\n"
      "Vary: Accept-Encoding\r\n"
      "\r\n"
      "{\"name\"}";
  Protocol::MixedCodec codec;
  for (size_t i = 0; i < rawResp.size(); i++) {
    *codec.Data() = rawResp[i];
    codec.Decode(1);
  }
  Protocol::HttpMessage *message = (Protocol::HttpMessage *)codec.GetMessage();
  bool result = message != nullptr;
  ASSERT_TRUE(result);

  Protocol::Packet pkt;
  codec.Encode(message, pkt);
  std::string temp((char *)pkt.DataRaw(), pkt.UseLen());
  ASSERT_EQ(rawResp, temp);
  delete message;
}

TEST_CASE(MixedCodec_Decode_Encode_MYSVR) {
  Protocol::MySvrMessage message;
  message.context_.set_log_id("666");
  message.context_.set_service_name("echo_server");
  message.context_.set_rpc_name("ping");
  std::string body = "hello";
  message.body_.Alloc(5);
  memmove(message.body_.Data(), body.data(), 5);
  message.body_.UpdateUseLen(5);

  Protocol::MixedCodec codec;
  Protocol::MySvrCodec mySvrCodec;
  Protocol::Packet pkt;
  bool result = mySvrCodec.Encode(&message, pkt);
  ASSERT_TRUE(result);

  uint8_t *data = pkt.DataRaw();
  for (size_t i = 0; i < pkt.UseLen(); i++) {
    *codec.Data() = *(data + i);
    codec.Decode(1);
  }
  Protocol::MySvrMessage *message1 = (Protocol::MySvrMessage *)codec.GetMessage();
  result = message1 != nullptr;
  ASSERT_TRUE(result);
  Protocol::Packet pkt2;
  ASSERT_TRUE(codec.Encode(message1, pkt2));
  ASSERT_EQ(pkt.UseLen(), pkt2.UseLen());
  ASSERT_EQ(0, memcmp(pkt.DataRaw(), pkt2.DataRaw(), pkt.UseLen()));

  ASSERT_EQ(message.context_.log_id(), message1->context_.log_id());
  ASSERT_EQ(message.context_.service_name(), message1->context_.service_name());
  ASSERT_EQ(message.context_.rpc_name(), message1->context_.rpc_name());
  ASSERT_EQ(message.body_.UseLen(), 5);
  ASSERT_EQ(message1->body_.UseLen(), 5);
  std::string body1((char *)message.body_.DataRaw(), message.body_.UseLen());
  std::string body2((char *)message1->body_.DataRaw(), message1->body_.UseLen());
  std::cout << "body1=" << body1 << std::endl;
  std::cout << "body2=" << body2 << std::endl;
  int ret = memcmp(message.body_.DataRaw(), message1->body_.DataRaw(), 5);
  ASSERT_EQ(ret, 0);
  delete message1;
}

TEST_CASE(MixedCodec_Data_Len) {
  Protocol::MixedCodec codec;
  ASSERT_EQ(codec.Len(), 1);
}
