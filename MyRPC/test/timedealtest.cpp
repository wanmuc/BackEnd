#include <unistd.h>

#include "../common/timedeal.hpp"
#include "unittestcore.h"

TEST_CASE(TimeDeal_TimeStat) {
  Common::TimeStat timeStat;
  sleep(1);
  uint64_t temp = timeStat.GetSpendTimeUs();
  std::cout << "SpendTimeUs = " << temp << std::endl;
  ASSERT_GT(temp, 1000000);
  sleep(2);
  temp = timeStat.GetSpendTimeUs();
  std::cout << "SpendTimeUs = " << temp << std::endl;
  ASSERT_GT(temp, 2000000);
}

TEST_CASE(TimeDeal_TimeStat_RestFALSE) {
  Common::TimeStat timeStat;
  sleep(1);
  uint64_t temp = timeStat.GetSpendTimeUs(false);
  std::cout << "SpendTimeUs = " << temp << std::endl;
  ASSERT_GT(temp, 1000000);
  sleep(2);
  temp = timeStat.GetSpendTimeUs();
  std::cout << "SpendTimeUs = " << temp << std::endl;
  ASSERT_GT(temp, 3000000);
}