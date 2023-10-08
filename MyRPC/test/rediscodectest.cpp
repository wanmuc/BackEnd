#include "../protocol/rediscodec.hpp"
#include "unittestcore.h"

TEST_CASE(RedisCodec_Encode_Auth) {
  Protocol::RedisCodec codec;
  Protocol::RedisCommand cmd;
  cmd.makeAuthCmd("backend");
  Protocol::Packet pkt;
  bool result = codec.Encode(&cmd, pkt);
  ASSERT_TRUE(result);
  std::string temp((char *)pkt.DataRaw(), pkt.UseLen());
  std::string temp1 = "*2\r\n$4\r\nAUTH\r\n$7\r\nbackend\r\n";
  ASSERT_EQ(temp, temp1);
}

TEST_CASE(RedisCodec_Encode_Get) {
  Protocol::RedisCodec codec;
  Protocol::RedisCommand cmd;
  cmd.makeGetCmd("key");
  Protocol::Packet pkt;
  bool result = codec.Encode(&cmd, pkt);
  ASSERT_TRUE(result);
  std::string temp((char *)pkt.DataRaw(), pkt.UseLen());
  std::string temp1 = "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n";
  ASSERT_EQ(temp, temp1);
}

TEST_CASE(RedisCodec_Encode_Del) {
  Protocol::RedisCodec codec;
  Protocol::RedisCommand cmd;
  cmd.makeDelCmd("key");
  Protocol::Packet pkt;
  bool result = codec.Encode(&cmd, pkt);
  ASSERT_TRUE(result);
  std::string temp((char *)pkt.DataRaw(), pkt.UseLen());
  std::string temp1 = "*2\r\n$3\r\nDEL\r\n$3\r\nkey\r\n";
  ASSERT_EQ(temp, temp1);
}

TEST_CASE(RedisCodec_Encode_SetNotExpire) {
  Protocol::RedisCodec codec;
  Protocol::RedisCommand cmd;
  cmd.makeSetCmd("key", "value");
  Protocol::Packet pkt;
  bool result = codec.Encode(&cmd, pkt);
  ASSERT_TRUE(result);
  std::string temp((char *)pkt.DataRaw(), pkt.UseLen());
  std::string temp1 = "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n";
  ASSERT_EQ(temp, temp1);
}

TEST_CASE(RedisCodec_Encode_SetExpire) {
  Protocol::RedisCodec codec;
  Protocol::RedisCommand cmd;
  cmd.makeSetCmd("key", "value", 300);
  Protocol::Packet pkt;
  bool result = codec.Encode(&cmd, pkt);
  ASSERT_TRUE(result);
  std::string temp((char *)pkt.DataRaw(), pkt.UseLen());
  std::string temp1 = "*5\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n$2\r\nEX\r\n$3\r\n300\r\n";
  ASSERT_EQ(temp, temp1);
}

TEST_CASE(RedisCodec_Encode_Incr) {
  Protocol::RedisCodec codec;
  Protocol::RedisCommand cmd;
  cmd.makeIncrCmd("key");
  Protocol::Packet pkt;
  bool result = codec.Encode(&cmd, pkt);
  ASSERT_TRUE(result);
  std::string temp((char *)pkt.DataRaw(), pkt.UseLen());
  std::string temp1 = "*2\r\n$4\r\nINCR\r\n$3\r\nkey\r\n";
  ASSERT_EQ(temp, temp1);
}

TEST_CASE(RedisCodec_Decode_OK) {
  Protocol::RedisCodec codec;
  std::string ok = "+OK\r\n";
  bool result = false;
  for (size_t i = 0; i < ok.size(); i++) {
    *codec.Data() = (uint8_t)ok[i];
    result = codec.Decode(1);  // 逐个字节进行解析
    ASSERT_TRUE(result);
  }
  Protocol::RedisReply *reply = (Protocol::RedisReply *)codec.GetMessage();
  result = (reply != nullptr);
  ASSERT_TRUE(result);
  ASSERT_EQ(reply->value_, "OK");
  delete reply;
}

TEST_CASE(RedisCodec_Decode_Errors) {
  Protocol::RedisCodec codec;
  std::string data = "-Error key not find\r\n";
  bool result = false;
  for (size_t i = 0; i < data.size(); i++) {
    *codec.Data() = (uint8_t)data[i];
    result = codec.Decode(1);  // 逐个字节进行解析
    ASSERT_TRUE(result);
  }
  Protocol::RedisReply *reply = (Protocol::RedisReply *)codec.GetMessage();
  result = (reply != nullptr);
  ASSERT_TRUE(result);
  ASSERT_EQ(reply->value_, "Error key not find");
  delete reply;
}

TEST_CASE(RedisCodec_Decode_Integers) {
  Protocol::RedisCodec codec;
  std::string data = ":666\r\n";
  bool result = false;
  for (size_t i = 0; i < data.size(); i++) {
    *codec.Data() = (uint8_t)data[i];
    result = codec.Decode(1);  // 逐个字节进行解析
    ASSERT_TRUE(result);
  }
  Protocol::RedisReply *reply = (Protocol::RedisReply *)codec.GetMessage();
  result = (reply != nullptr);
  ASSERT_TRUE(result);
  ASSERT_EQ(reply->value_, "666");
  delete reply;
}

TEST_CASE(RedisCodec_Decode_BulkStrings_Null) {
  Protocol::RedisCodec codec;
  std::string data = "$-1\r\n";
  bool result = false;
  for (size_t i = 0; i < data.size(); i++) {
    *codec.Data() = (uint8_t)data[i];
    result = codec.Decode(1);  // 逐个字节进行解析
    ASSERT_TRUE(result);
  }
  Protocol::RedisReply *reply = (Protocol::RedisReply *)codec.GetMessage();
  result = (reply != nullptr);
  ASSERT_TRUE(result);
  ASSERT_EQ(reply->value_, "");
  ASSERT_TRUE(reply->IsNull());
  delete reply;
}

TEST_CASE(RedisCodec_Decode_BulkStrings_Empty) {
  Protocol::RedisCodec codec;
  std::string data = "$0\r\n\r\n";
  bool result = false;
  for (size_t i = 0; i < data.size(); i++) {
    *codec.Data() = (uint8_t)data[i];
    result = codec.Decode(1);  // 逐个字节进行解析
    ASSERT_TRUE(result);
  }
  Protocol::RedisReply *reply = (Protocol::RedisReply *)codec.GetMessage();
  result = (reply != nullptr);
  ASSERT_TRUE(result);
  ASSERT_EQ(reply->value_, "");
  ASSERT_FALSE(reply->IsNull());
  delete reply;
}

TEST_CASE(RedisCodec_Decode_BulkStrings_HasValue) {
  Protocol::RedisCodec codec;
  std::string data = "$5\r\nhello\r\n";
  bool result = false;
  for (size_t i = 0; i < data.size(); i++) {
    *codec.Data() = (uint8_t)data[i];
    result = codec.Decode(1);  // 逐个字节进行解析
    ASSERT_TRUE(result);
  }
  Protocol::RedisReply *reply = (Protocol::RedisReply *)codec.GetMessage();
  result = (reply != nullptr);
  ASSERT_TRUE(result);
  ASSERT_EQ(reply->value_, "hello");
  ASSERT_FALSE(reply->IsNull());
  delete reply;
}