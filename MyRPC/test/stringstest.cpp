#include "../common/strings.hpp"
#include "unittestcore.h"

TEST_CASE(Strings_ltrim) {
  std::string str = " aaa   ";
  Common::Strings::ltrim(str);
  ASSERT_EQ(str, "aaa   ");
}
TEST_CASE(Strings_rtrim) {
  std::string str = " aaa   ";
  Common::Strings::rtrim(str);
  ASSERT_EQ(str, " aaa");
}
TEST_CASE(Strings_trim) {
  std::string str = " aaa   ";
  Common::Strings::trim(str);
  ASSERT_EQ(str, "aaa");
}
TEST_CASE(Strings_Split) {
  std::string str = "123|345|567";
  std::vector<std::string> vec;
  Common::Strings::Split(str, "|", vec);
  ASSERT_EQ(vec.size(), 3);
  ASSERT_EQ(vec[0], "123");
  ASSERT_EQ(vec[2], "567");

  str = "";
  vec.clear();
  Common::Strings::Split(str, "|", vec);
  ASSERT_EQ(vec.size(), 0);

  str = "123|345|567|";
  vec.clear();
  Common::Strings::Split(str, "|", vec);
  ASSERT_EQ(vec.size(), 3);
  ASSERT_EQ(vec[0], "123");
  ASSERT_EQ(vec[2], "567");
}
TEST_CASE(Strings_Join) {
  std::vector<std::string> vec = {"aa", "bb", "cc"};
  std::string result;
  result = Common::Strings::Join(vec, "|");
  ASSERT_EQ(result, "aa|bb|cc");
}
TEST_CASE(Strings_StrFormat) {
  std::string name = "hello";
  std::string result = Common::Strings::StrFormat((char *)"hello%d%s", 1, name.c_str());
  ASSERT_EQ(result, "hello1hello");
}
TEST_CASE(Strings_ToLower) {
  std::string str = "ABC";
  Common::Strings::ToLower(str);
  ASSERT_EQ(str, "abc");
}