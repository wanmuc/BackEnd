#include "../common/percentile.hpp"
#include "unittestcore.h"

TEST_CASE(Percentile_All) {
  bool result;
  double pctValue;
  Common::Percentile pct(8);
  std::vector<int64_t> temp{2, 10, 5, 8, 2, 5, 9, 10};
  for (size_t i = 0; i < temp.size(); i++) {
    pct.Stat("test", temp[i]);
    if (i != temp.size() - 1) {
      result = pct.GetPercentile("test", 0.5, pctValue);
      ASSERT_FALSE(result);
    } else {
      result = pct.GetPercentile("test", 0.5, pctValue);
      ASSERT_TRUE(result);
    }
  }
  result = pct.GetPercentile("test", 0.1, pctValue);
  ASSERT_TRUE(result);
  std::cout << "pct10 = " << pctValue << std::endl;
  result = pct.GetPercentile("test", 0.5, pctValue);
  ASSERT_TRUE(result);
  std::cout << "pct50 = " << pctValue << std::endl;
  result = pct.GetPercentile("test", 0.9, pctValue);
  ASSERT_TRUE(result);
  std::cout << "pct90 = " << pctValue << std::endl;

  Common::Percentile pct2(100);
  for (size_t i = 0; i < 100; i++) {
    pct2.Stat("test", i + 1);
  }
  result = pct2.GetPercentile("test2", 0.1, pctValue);
  ASSERT_FALSE(result);
  result = pct2.GetPercentile("test", 0.111, pctValue);
  ASSERT_TRUE(result);
  std::cout << "pct11.1 = " << pctValue << std::endl;
  result = pct2.GetPercentile("test", 0.555, pctValue);
  ASSERT_TRUE(result);
  std::cout << "pct55.5 = " << pctValue << std::endl;
  result = pct2.GetPercentile("test", 0.999, pctValue);
  ASSERT_TRUE(result);
  std::cout << "pct99.9 = " << pctValue << std::endl;

  Common::Percentile pct3(10);
  for (size_t i = 1; i <= 10; i++) {
    pct3.Stat("test3", i * 10);
  }
  result = pct3.GetPercentile("test3", 0.90, pctValue);
  ASSERT_TRUE(result);
  std::cout << "pct90 = " << pctValue << std::endl;
}