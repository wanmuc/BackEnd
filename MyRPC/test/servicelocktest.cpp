#include "../common/servicelock.hpp"
#include "unittestcore.h"

TEST_CASE(ServiceLock_lock) {
  bool result = Common::ServiceLock::lock("/home/backend/lock/subsys/" + Common::Utils::GetSelfName());
  ASSERT_TRUE(result);
  result = Common::ServiceLock::lock("/home/backend/lock/subsys/" + Common::Utils::GetSelfName());
  ASSERT_TRUE(result);
}