#include "../common/cmdline.h"
#include "unittestcore.h"

void Usage() { std::cout << "unit_test" << std::endl; }
TEST_CASE(CmdLine_All) {
  Common::CmdLine::SetUsage(Usage);
  char str0[] = "UnitTest";
  char str1[] = "-d=1";
  char str2[] = "-type";
  char str3[] = "MySvr";
  char str4[] = "-len";
  char str5[] = "666";
  char str6[] = "-op";
  char str7[] = "get";
  char str8[] = "-score";
  char str9[] = "100";
  char* argv[] = {str0, str1, str2, str3, str4, str5, str6, str7, str8, str9};
  bool daemon = false;
  bool daemon2 = false;
  int64_t len = 0;
  int64_t score = 0;
  std::string type;
  std::string op;
  Common::CmdLine::BoolOpt(&daemon, "d");
  Common::CmdLine::BoolOpt(&daemon2, "D");
  Common::CmdLine::Int64Opt(&len, "len", 0);
  Common::CmdLine::StrOpt(&type, "type", "");
  Common::CmdLine::Int64OptRequired(&score, "score");
  Common::CmdLine::StrOptRequired(&op, "op");
  Common::CmdLine::Parse(10, argv);
  ASSERT_TRUE(daemon);
  ASSERT_FALSE(daemon2);
  ASSERT_EQ(len, 666);
  ASSERT_EQ(type, "MySvr");
  ASSERT_EQ(score, 100);
  ASSERT_EQ(op, "get");
}