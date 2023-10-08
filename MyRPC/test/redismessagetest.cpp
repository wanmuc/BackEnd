#include "../protocol/redismessage.hpp"
#include "unittestcore.h"

TEST_CASE(RedisCommand_Get) {
  Protocol::RedisCommand cmd;
  cmd.makeGetCmd("123");
  std::string str;
  cmd.GetOut(str);
  ASSERT_EQ(str, "*2\r\n$3\r\nGET\r\n$3\r\n123\r\n");
}

TEST_CASE(RedisCommand_Del) {
  Protocol::RedisCommand cmd;
  cmd.makeDelCmd("1234");
  std::string str;
  cmd.GetOut(str);
  ASSERT_EQ(str, "*2\r\n$3\r\nDEL\r\n$4\r\n1234\r\n");
}

TEST_CASE(RedisCommand_Auth) {
  Protocol::RedisCommand cmd;
  cmd.makeAuthCmd("12345");
  std::string str;
  cmd.GetOut(str);
  ASSERT_EQ(str, "*2\r\n$4\r\nAUTH\r\n$5\r\n12345\r\n");
}

TEST_CASE(RedisCommand_Set) {
  Protocol::RedisCommand cmd;
  cmd.makeSetCmd("name", "test");
  std::string str;
  cmd.GetOut(str);
  ASSERT_EQ(str, "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$4\r\ntest\r\n");
}

TEST_CASE(RedisCommand_SetWithEx) {
  Protocol::RedisCommand cmd;
  cmd.makeSetCmd("name", "test", 10);
  std::string str;
  cmd.GetOut(str);
  ASSERT_EQ(str, "*5\r\n$3\r\nSET\r\n$4\r\nname\r\n$4\r\ntest\r\n$2\r\nEX\r\n$2\r\n10\r\n");
}

TEST_CASE(RedisCommand_Incr) {
  Protocol::RedisCommand cmd;
  cmd.makeIncrCmd("123456");
  std::string str;
  cmd.GetOut(str);
  ASSERT_EQ(str, "*2\r\n$4\r\nINCR\r\n$6\r\n123456\r\n");
}

TEST_CASE(RedisReply_OK) {
  Protocol::RedisReply reply;
  reply.value_ = "OK";
  reply.type_ = Protocol::SIMPLE_STRINGS;
  ASSERT_TRUE(reply.IsOk());
  ASSERT_FALSE(reply.IsNull());
}

TEST_CASE(RedisReply_Error) {
  Protocol::RedisReply reply;
  reply.value_ = "long type";
  reply.type_ = Protocol::ERRORS;
  ASSERT_TRUE(reply.IsError());
  ASSERT_FALSE(reply.IsNull());
  ASSERT_EQ(reply.Value(), "long type");
}

TEST_CASE(RedisReply_Integers) {
  Protocol::RedisReply reply;
  reply.value_ = "123456789";
  reply.type_ = Protocol::INTEGERS;
  ASSERT_FALSE(reply.IsNull());
  ASSERT_EQ(reply.IntValue(), 123456789);
}