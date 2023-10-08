#include "../common/log.hpp"
#include "../common/utils.hpp"
#include "../protocol/base.pb.h"
#include "unittestcore.h"

TEST_CASE(Utils_GetSelfName) { ASSERT_EQ(Common::Utils::GetSelfName(), "UnitTest"); }
TEST_CASE(Utils_GetIpStr) {
  ASSERT_EQ(Common::Utils::GetIpStr("lo"), "127000000001");
  ASSERT_EQ(Common::Utils::GetIpStr("eth0"), "127000000001");
}
TEST_CASE(Utils_GetAddr) {
  ASSERT_EQ(Common::Utils::GetAddr("lo"), htonl(INADDR_LOOPBACK));
  ASSERT_EQ(Common::Utils::GetAddr("any"), htonl(INADDR_ANY));
  ASSERT_EQ(Common::Utils::GetAddr("eth0"), htonl(INADDR_LOOPBACK));
}
TEST_CASE(Utils_GetLogId) { ASSERT_GT(Common::Logger::GetLogId().size(), 1); }
TEST_CASE(Utils_GetNProcs) { ASSERT_GT(Common::Utils::GetNProcs(), 1); }
