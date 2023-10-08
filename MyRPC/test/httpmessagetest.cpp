#include "../protocol/httpmessage.hpp"
#include "unittestcore.h"

TEST_CASE(HttpMessage_SetAndGetHeader) {
  Protocol::HttpMessage httpMessage;
  httpMessage.SetHeader("Host", "127.0.0.1");
  ASSERT_EQ(httpMessage.GetHeader("Host"), "127.0.0.1");
  ASSERT_EQ(httpMessage.GetHeader("host"), "");
}

TEST_CASE(HttpMessage_SetBody) {
  Protocol::HttpMessage httpMessage;
  httpMessage.SetBody(R"({"name":"test"})");
  ASSERT_EQ(httpMessage.GetHeader("Content-Type"), "application/json");
  ASSERT_EQ(httpMessage.GetHeader("Content-Length"), "15");
}

TEST_CASE(HttpMessage_SetStatusCode) {
  Protocol::HttpMessage httpMessage;
  httpMessage.SetStatusCode(Protocol::OK);
  std::string method;
  std::string url;
  httpMessage.GetMethodAndUrl(method, url);
  ASSERT_EQ(method, "HTTP/1.1");
  ASSERT_EQ(url, "200");
}
