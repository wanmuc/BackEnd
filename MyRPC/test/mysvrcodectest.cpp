#include "../protocol/mysvrcodec.hpp"
#include "unittestcore.h"

TEST_CASE(MySvrCodec_Encode_Decode) {
  Protocol::MySvrMessage message;
  message.context_.set_log_id("666");
  message.context_.set_service_name("echo_server");
  message.context_.set_rpc_name("ping");
  std::string body = "hello";
  message.body_.Alloc(5);
  memmove(message.body_.Data(), body.data(), 5);
  message.body_.UpdateUseLen(5);

  Protocol::MySvrCodec codec;

  for (size_t i = 0; i < 5; i++) {
    Protocol::Packet pkt;
    bool result = codec.Encode(&message, pkt);
    ASSERT_TRUE(result);

    uint8_t *data = pkt.DataRaw();
    for (size_t j = 0; j < pkt.UseLen(); j++) {
      *codec.Data() = *(data + j);
      ASSERT_TRUE(codec.Decode(1));  // 逐个字节进行解析
    }
    Protocol::MySvrMessage *message1 = (Protocol::MySvrMessage *)codec.GetMessage();
    result = message1 != nullptr;
    ASSERT_TRUE(result);

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
}

TEST_CASE(MySvrCodec_SetLimit1) {
  Protocol::MySvrMessage message;
  message.context_.set_log_id("666");
  message.context_.set_service_name("echo_server");
  message.context_.set_rpc_name("ping");
  std::string body = "hello";
  message.body_.Alloc(5);
  memmove(message.body_.Data(), body.data(), 5);
  message.body_.UpdateUseLen(5);

  Protocol::MySvrCodec codec;
  Protocol::Packet pkt;
  codec.SetLimit(5, 10000);
  bool result = codec.Encode(&message, pkt);
  ASSERT_TRUE(result);
  uint8_t *data = pkt.DataRaw();
  for (size_t j = 0; j < pkt.UseLen(); j++) {
    *codec.Data() = *(data + j);
    if (j < 7) {
      ASSERT_TRUE(codec.Decode(1));
    } else {
      ASSERT_FALSE(codec.Decode(1));
      break;
    }
  }
}

TEST_CASE(MySvrCodec_SetLimit2) {
  Protocol::MySvrMessage message;
  message.context_.set_log_id("666");
  message.context_.set_service_name("echo_server");
  message.context_.set_rpc_name("ping");
  std::string body = "hello";
  message.body_.Alloc(5);
  memmove(message.body_.Data(), body.data(), 5);
  message.body_.UpdateUseLen(5);

  Protocol::MySvrCodec codec;
  Protocol::Packet pkt;
  codec.SetLimit(10000, 4);
  bool result = codec.Encode(&message, pkt);
  ASSERT_TRUE(result);
  uint8_t *data = pkt.DataRaw();
  for (size_t j = 0; j < pkt.UseLen(); j++) {
    *codec.Data() = *(data + j);
    if (j < 7) {
      ASSERT_TRUE(codec.Decode(1));
    } else {
      ASSERT_FALSE(codec.Decode(1));
      break;
    }
  }
}