#pragma once

#include <sys/time.h>

#include <string>

namespace Common {
class TimeStat {
 public:
  TimeStat() { gettimeofday(&begin, NULL); }
  int64_t GetSpendTimeUs(bool reset = true) {
    struct timeval temp;
    struct timeval current;
    gettimeofday(&current, NULL);
    temp = current;
    temp.tv_sec -= begin.tv_sec;
    temp.tv_usec -= begin.tv_usec;
    if (temp.tv_usec < 0) {
      temp.tv_sec -= 1;
      temp.tv_usec += 1000000;
    }
    if (reset) begin = current;
    return temp.tv_sec * 1000000 + temp.tv_usec;  //计算运行的时间，单位微秒
  }

 private:
  struct timeval begin;
};
class TimeFormat {
 public:
  static std::string GetTimeStr(const char *format, bool hasUSec = false) {
    struct timeval curTime;
    char temp[100] = {0};
    char timeStr[100] = {0};
    gettimeofday(&curTime, NULL);
    strftime(temp, 99, format, localtime(&curTime.tv_sec));
    if (hasUSec) {
      snprintf(timeStr, 99, "%s:%06ld", temp, curTime.tv_usec);
      return std::string(timeStr);
    }
    return std::string(temp);
  }
};
}  // namespace Common