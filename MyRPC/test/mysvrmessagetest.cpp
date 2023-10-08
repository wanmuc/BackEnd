#include "../protocol/mysvrmessage.hpp"
#include "unittestcore.h"

TEST_CASE(MySvrMessage_IsFastResp) {
  Protocol::MySvrMessage message;
  message.EnableFastResp();
  ASSERT_TRUE(message.IsFastResp());
}

TEST_CASE(MySvrMessage_IsOneway) {
  Protocol::MySvrMessage message;
  message.EnableOneway();
  ASSERT_TRUE(message.IsOneway());
}

TEST_CASE(MySvrMessage_BodyIsJson) {
  Protocol::MySvrMessage message;
  message.BodyEnableJson();
  ASSERT_TRUE(message.BodyIsJson());
}

TEST_CASE(MySvrMessage_StatusCode) {
  Protocol::MySvrMessage message;
  ASSERT_EQ(message.StatusCode(), 0);
  ASSERT_EQ(message.Message(), "success");
  message.context_.set_status_code(666);
  ASSERT_EQ(message.StatusCode(), 666);
  ASSERT_EQ(message.Message(), "unknown");
}

TEST_CASE(MySvrMessage_CopyFrom) {
  Protocol::MySvrMessage message;
  Protocol::MySvrMessage message2;
  ASSERT_EQ(message.StatusCode(), 0);
  ASSERT_EQ(message.Message(), "success");
  message.context_.set_status_code(666);
  ASSERT_EQ(message.StatusCode(), 666);
  ASSERT_EQ(message.Message(), "unknown");
  message2.CopyFrom(message);
  ASSERT_EQ(message2.StatusCode(), 666);
  ASSERT_EQ(message2.Message(), "unknown");
  ASSERT_FALSE(message2.IsFastResp());
  ASSERT_FALSE(message2.IsOneway());
  ASSERT_FALSE(message2.BodyIsJson());
}
