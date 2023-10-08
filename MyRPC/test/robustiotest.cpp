#include "../common/robustio.hpp"
#include "unittestcore.h"

TEST_CASE(RobustIo_All) {
  int fd = open("/home/backend/log/UnitTest/UnitTest.log", O_RDONLY);
  Common::RobustIo robustIo(fd);
  ASSERT_TRUE(robustIo.RestartAgain(EINTR));
  ASSERT_FALSE(robustIo.RestartAgain(EAGAIN));
  ASSERT_FALSE(robustIo.RestartAgain(EWOULDBLOCK));
  robustIo.SetNotBlock();
  robustIo.SetTimeOut(1, 0);
  ASSERT_TRUE(robustIo.RestartAgain(EINTR));
  ASSERT_TRUE(robustIo.RestartAgain(EAGAIN));
  ASSERT_TRUE(robustIo.RestartAgain(EWOULDBLOCK));
  uint8_t data[50];
  ASSERT_EQ(robustIo.Read(data, 50), 50);
}