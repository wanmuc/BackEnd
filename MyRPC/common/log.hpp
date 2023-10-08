#pragma once
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#include "robustio.hpp"
#include "singleton.hpp"
#include "strings.hpp"
#include "timedeal.hpp"
#include "utils.hpp"

namespace Common {
enum LogLevel {  //日志输出级别
  LEVEL_TRACE = 0,
  LEVEL_DEBUG = 1,
  LEVEL_INFO = 2,
  LEVEL_WARN = 3,
  LEVEL_ERROR = 4,
};

class Logger {  //日志文件类
 public:
  Logger() {
    std::string programName = Utils::GetSelfName();
    const char *cStr = programName.c_str();
    std::string fileName = Strings::StrFormat((char *)"/home/backend/log/%s/%s.log", cStr, cStr);
    fd_ = open(fileName.c_str(), O_APPEND | O_CREAT | O_WRONLY,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);  //追加写的方式打开文件
    assert(fd_ > 0);
    srand(time(0));
  }
  void SetLevel(LogLevel level) { level_ = level; }
  void Log(std::string logId, LogLevel level, char *format, ...) {
    if (level < level_) return;
    int32_t ret = 0;
    char *buf = (char *)malloc(1024);
    va_list plist;
    va_start(plist, format);
    ret = vsnprintf(buf, 1024, format, plist);
    va_end(plist);
    assert(ret > 0);
    if (ret >= 1024) {  //缓冲区长度不足，需要重新分配内存
      buf = (char *)realloc(buf, ret + 1);
      va_start(plist, format);
      ret = vsnprintf(buf, ret + 1, format, plist);
      va_end(plist);
    }
    if (logId == "") {
      logId = GetLogId();
    }
    std::string timeStr = TimeFormat::GetTimeStr("%F %T", true);
    std::string logMsg =
        levelStr(level) + " " + timeStr + " " + std::to_string(getpid()) + "," + logId + " " + buf + "\n";
    free(buf);
    RobustIo io(fd_);
    io.Write((uint8_t *)logMsg.data(), logMsg.size());
  }
  static std::string GetLogId() {
    static std::string ip = Common::Utils::GetIpStr("eth0");  //默认取eth0的ip
    std::string curTime = TimeFormat::GetTimeStr("%Y%m%d%H%M%S");
    return curTime + ip + std::to_string(rand() % 1000000);
  }

 private:
  std::string levelStr(LogLevel level) {
    if (LEVEL_TRACE == level) return "[TRACE]";
    if (LEVEL_DEBUG == level) return "[DEBUG]";
    if (LEVEL_INFO == level) return "[INFO]";
    if (LEVEL_WARN == level) return "[WARN]";
    if (LEVEL_ERROR == level) return "[ERROR]";
    return "UNKNOWN";
  }

 protected:
  LogLevel level_{LEVEL_TRACE};  //日志级别
  int fd_{-1};                   //文件句柄
};
}  // namespace Common

#define LOGGER Common::Singleton<Common::Logger>::Instance()
#define FILENAME(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x
#define TRACE(format, ...)                                                                                      \
  LOGGER.Log("", Common::LEVEL_TRACE, (char *)"(%s:%s:%d):" format, FILENAME(__FILE__), __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define DEBUG(format, ...)                                                                                      \
  LOGGER.Log("", Common::LEVEL_DEBUG, (char *)"(%s:%s:%d):" format, FILENAME(__FILE__), __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define INFO(format, ...)                                                                                      \
  LOGGER.Log("", Common::LEVEL_INFO, (char *)"(%s:%s:%d):" format, FILENAME(__FILE__), __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define WARN(format, ...)                                                                                      \
  LOGGER.Log("", Common::LEVEL_WARN, (char *)"(%s:%s:%d):" format, FILENAME(__FILE__), __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define ERROR(format, ...)                                                                                      \
  LOGGER.Log("", Common::LEVEL_ERROR, (char *)"(%s:%s:%d):" format, FILENAME(__FILE__), __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define CTX_TRACE(ctx, format, ...)                                              \
  LOGGER.Log(ctx.log_id(), Common::LEVEL_TRACE, (char *)"(%d:%s:%s:%d):" format, \
             MyCoroutine::ScheduleGetRunCid(SCHEDULE), FILENAME(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define CTX_DEBUG(ctx, format, ...)                                              \
  LOGGER.Log(ctx.log_id(), Common::LEVEL_DEBUG, (char *)"(%d:%s:%s:%d):" format, \
             MyCoroutine::ScheduleGetRunCid(SCHEDULE), FILENAME(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define CTX_INFO(ctx, format, ...)                                              \
  LOGGER.Log(ctx.log_id(), Common::LEVEL_INFO, (char *)"(%d:%s:%s:%d):" format, \
             MyCoroutine::ScheduleGetRunCid(SCHEDULE), FILENAME(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define CTX_WARN(ctx, format, ...)                                              \
  LOGGER.Log(ctx.log_id(), Common::LEVEL_WARN, (char *)"(%d:%s:%s:%d):" format, \
             MyCoroutine::ScheduleGetRunCid(SCHEDULE), FILENAME(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define CTX_ERROR(ctx, format, ...)                                              \
  LOGGER.Log(ctx.log_id(), Common::LEVEL_ERROR, (char *)"(%d:%s:%s:%d):" format, \
             MyCoroutine::ScheduleGetRunCid(SCHEDULE), FILENAME(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
