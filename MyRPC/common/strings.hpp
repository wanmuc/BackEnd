#pragma once

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include <algorithm>
#include <string>
#include <vector>

namespace Common {
class Strings {
 public:
  static void ltrim(std::string &str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(" "));
  }
  static void rtrim(std::string &str) {
    if (str.empty()) return;
    str.erase(str.find_last_not_of(" ") + 1);
  }
  static void trim(std::string &str) {
    ltrim(str);
    rtrim(str);
  }
  static void Split(std::string &str, std::string sep, std::vector<std::string> &result) {
    if (str == "") return;
    std::string::size_type prePos = 0;
    std::string::size_type curPos = str.find(sep);
    while (std::string::npos != curPos) {
      std::string subStr = str.substr(prePos, curPos - prePos);
      if (subStr != "") {  //非空串才插入
        result.push_back(subStr);
      }
      prePos = curPos + sep.size();
      curPos = str.find(sep, prePos);
    }
    if (prePos != str.length()) {
      result.push_back(str.substr(prePos));
    }
  }
  static std::string Join(std::vector<std::string> &strs, std::string sep) {
    std::string result;
    for (size_t i = 0; i < strs.size(); ++i) {
      result += strs[i];
      if (i != strs.size() - 1) {
        result += sep;
      }
    }
    return result;
  }
  static std::string StrFormat(char *format, ...) {
    char *buf = (char *)malloc(1024);
    va_list plist;
    va_start(plist, format);
    int ret = vsnprintf(buf, 1024, format, plist);
    va_end(plist);
    assert(ret > 0);
    if (ret >= 1024) {  //缓冲区长度不足，需要重新分配内存
      buf = (char *)realloc(buf, ret + 1);
      va_start(plist, format);
      ret = vsnprintf(buf, ret + 1, format, plist);
      va_end(plist);
    }
    std::string result(buf);
    free(buf);
    return result;
  }
  static void ToLower(std::string &str) { transform(str.begin(), str.end(), str.begin(), ::tolower); }
};  // namespace Strings
}  // namespace Common
