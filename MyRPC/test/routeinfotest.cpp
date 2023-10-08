#include "../core/routeinfo.hpp"
#include "unittestcore.h"

TEST_CASE(RouteInfo_All) {
  Core::TimeOut timeOut;
  Core::Route route;
  Core::RouteInfo routeInfo;
  routeInfo.SetExpireTime(1);
  bool get = routeInfo.GetRoute("ECho", route, timeOut);
  ASSERT_TRUE(get);
  sleep(2);
  get = routeInfo.GetRoute("ECho", route, timeOut, 100);
  ASSERT_TRUE(get);
  get = routeInfo.GetRoute("ECho2", route, timeOut, 100);
  ASSERT_FALSE(get);
}