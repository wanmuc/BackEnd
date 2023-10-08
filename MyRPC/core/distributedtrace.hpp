#pragma once

#include <assert.h>

#include <vector>

#include "../common/log.hpp"
#include "../protocol/base.pb.h"
#include "../protocol/mysvrmessage.hpp"
#include "coroutinelocal.hpp"

extern Core::CoroutineLocal<MySvr::Base::Context> ReqCtx;

namespace Core {
using namespace MySvr::Base;
class DistributedTrace {
 public:
  static void PrintTraceInfo(Context &ctx, int stackId, int depth, bool isLast = false, bool outputIsLog = true) {
    if (stackId <= 0) {
      WARN("invalid stackId[%d]", stackId);
      return;
    }
    std::string message;
    std::string servicePrefix = "";
    if (1 == stackId) {
      servicePrefix = "Direct.";
    }
    const TraceStack traceInfo = getTraceInfo(ctx, stackId);
    message = Common::Strings::StrFormat(
        (char *)"%s[%d]%s%s.%s-[%ldus,%d,%d,%s]", getPrefix(depth, isLast).c_str(), traceInfo.current_id(),
        servicePrefix.c_str(), traceInfo.service_name().c_str(), traceInfo.rpc_name().c_str(), traceInfo.spend_us(),
        traceInfo.is_batch(), traceInfo.status_code(), traceInfo.message().c_str());
    if (outputIsLog) {
      CTX_TRACE(ctx, "%s", message.c_str());
    } else {
      std::cout << message << std::endl;
    }
    std::vector<TraceStack> child;
    getChildTraceInfos(ctx, child, stackId);
    for (size_t i = 0; i < child.size(); i++) {
      PrintTraceInfo(ctx, child[i].current_id(), depth + 1, i == child.size() - 1, outputIsLog);
    }
  }
  static void InitTraceInfo(Context &ctx) {
    if (ctx.log_id() == "") {
      ctx.set_log_id(Common::Logger::GetLogId());
    }
    ReqCtx.Set(ctx);
    /*
     * 没有设置parent_stack_id（调用方）时，则使用stack_alloc_id设置，此时parent_stack_id被设置为0。
     * stack_alloc_id的值从0开始，parent_stack_id值为0，表示该服务节点为分布式调用的起点。
     */
    if (0 == ReqCtx.Get().parent_stack_id()) {
      ReqCtx.Get().set_parent_stack_id(ReqCtx.Get().stack_alloc_id());
    }
    ReqCtx.Get().set_current_stack_id(ReqCtx.Get().stack_alloc_id() + 1);  // 分配的分布式调用栈id（每次自动加1）
    ReqCtx.Get().set_stack_alloc_id(ReqCtx.Get().stack_alloc_id() + 1);  // 更新分布式调用栈id
  }
  // 用于MyRPC框架，合入当前rpc接口分布式栈信息。
  static void AddTraceInfo(int64_t spendTimeUs, int32_t statusCode, std::string message) {
    auto trace_stack = ReqCtx.Get().add_trace_stack();
    trace_stack->set_rpc_name(ReqCtx.Get().rpc_name());
    trace_stack->set_service_name(ReqCtx.Get().service_name());
    trace_stack->set_status_code(statusCode);
    trace_stack->set_message(message);
    trace_stack->set_spend_us(spendTimeUs);
    trace_stack->set_is_batch(MyCoroutine::CoroutineIsInBatch(SCHEDULE));
    trace_stack->set_parent_id(ReqCtx.Get().parent_stack_id());
    trace_stack->set_current_id(ReqCtx.Get().current_stack_id());
  }
  // 用于合入非MyRPC调用的分布式调用栈信息，比如对Redis的调用。
  static void AddTraceInfo(std::string serviceName, std::string rpcName, int64_t spendTimeUs, int32_t statusCode,
                           std::string message) {
    auto trace_stack = ReqCtx.Get().add_trace_stack();
    trace_stack->set_rpc_name(rpcName);
    trace_stack->set_service_name(serviceName);
    trace_stack->set_status_code(statusCode);
    trace_stack->set_message(message);
    trace_stack->set_spend_us(spendTimeUs);
    trace_stack->set_is_batch(MyCoroutine::CoroutineIsInBatch(SCHEDULE));
    trace_stack->set_parent_id(ReqCtx.Get().current_stack_id());
    trace_stack->set_current_id(ReqCtx.Get().stack_alloc_id() + 1);
    ReqCtx.Get().set_stack_alloc_id(ReqCtx.Get().stack_alloc_id() + 1);
  }
  // 用于MyRPC框架，合入依赖的rpc接口分布式栈信息。
  static void MergeTraceInfo(Context &ctx) {
    ReqCtx.Get().set_stack_alloc_id(ctx.stack_alloc_id());  // 更新调用栈分配的id（下游会改变这个分布式调用栈id的值）
    for (int i = 0; i < ctx.trace_stack_size(); i++) {
      ReqCtx.Get().add_trace_stack()->CopyFrom(ctx.trace_stack(i));
    }
  }

 private:
  static std::string getPrefix(int depth, bool isLast) {
    std::string prefix = "";
    if (0 == depth) return prefix;
    depth = (depth * 2) - 1;
    while (depth--) {
      prefix += " ";
    }
    if (isLast) {
      prefix += "└";
    } else {
      prefix += "├";
    }
    return prefix;
  }
  static const TraceStack getTraceInfo(Context &ctx, int stackId) {
    for (int i = 0; i < ctx.trace_stack_size(); i++) {
      if (ctx.trace_stack(i).current_id() == stackId) {
        return ctx.trace_stack(i);
      }
    }
    assert(0);
  }
  static void getChildTraceInfos(Context &ctx, std::vector<TraceStack> &childTraceInfo, int parentId) {
    for (int i = 0; i < ctx.trace_stack_size(); i++) {
      if (ctx.trace_stack(i).parent_id() == parentId) {
        childTraceInfo.push_back(ctx.trace_stack(i));
      }
    }
  }
};
}  // namespace Core