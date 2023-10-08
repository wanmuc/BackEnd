#include "../common/convert.hpp"
#include "../common/log.hpp"
#include "../protocol/base.pb.h"
#include "unittestcore.h"

TEST_CASE(Convert_Pb2JsonStr) {
  MySvr::Base::Context ctx;
  ctx.set_log_id(Common::Logger::GetLogId());
  ctx.set_service_name("EchoServer");
  ctx.set_rpc_name("Ping");
  ctx.set_status_code(0);
  auto trace_stack = ctx.add_trace_stack();
  trace_stack->set_parent_id(100);
  trace_stack->set_message("success");
  std::string jsonStr;
  ASSERT_TRUE(Common::Convert::Pb2JsonStr(ctx, jsonStr, true));
  std::cout << "jsonStr = " << jsonStr << std::endl;
}
TEST_CASE(Convert_JsonStr2Pb) {
  MySvr::Base::Context ctx;
  ctx.set_log_id(Common::Logger::GetLogId());
  ctx.set_service_name("EchoServer");
  ctx.set_rpc_name("Ping");
  ctx.set_status_code(0);
  auto trace_stack = ctx.add_trace_stack();
  trace_stack->set_parent_id(100);
  trace_stack->set_message("success");
  std::string jsonStr;
  ASSERT_TRUE(Common::Convert::Pb2JsonStr(ctx, jsonStr, true));
  std::cout << "jsonStr = " << jsonStr << std::endl;
  MySvr::Base::Context ctx2;
  ASSERT_TRUE(Common::Convert::JsonStr2Pb(jsonStr, ctx2));
  ASSERT_EQ(ctx.log_id(), ctx2.log_id());
  ASSERT_EQ(ctx.service_name(), ctx2.service_name());
}
TEST_CASE(Convert_Pb2Json) {
  MySvr::Base::Context ctx;
  ctx.set_log_id(Common::Logger::GetLogId());
  ctx.set_service_name("EchoServer");
  ctx.set_rpc_name("Ping");
  ctx.set_status_code(0);
  Json::Value value;
  ASSERT_TRUE(Common::Convert::Pb2Json(ctx, value));
  ASSERT_EQ(value["service_name"].asString(), "EchoServer");
}
TEST_CASE(Convert_Json2Pb) {
  Json::Value value;
  value["service_name"] = "EchoServer";
  MySvr::Base::Context ctx;
  ASSERT_TRUE(Common::Convert::Json2Pb(value, ctx));
  ASSERT_EQ(ctx.service_name(), "EchoServer");
}
