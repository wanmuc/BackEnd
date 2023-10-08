#pragma once

#include "../common/defer.hpp"
#include "../common/statuscode.hpp"
#include "../protocol/mixedcodec.hpp"
#include "client.hpp"
#include "distributedtrace.hpp"

namespace Core {
class MySvrClient : public Client {
 public:
  int PushCall(google::protobuf::Message &pbMessage) {
    Protocol::MySvrMessage mySvrMessage;
    createMySvrByPb(mySvrMessage, pbMessage);
    mySvrMessage.EnableOneway();  // 开启oneway模式
    PushCallRaw(mySvrMessage);
    return mySvrMessage.StatusCode();
  }
  int RpcCall(google::protobuf::Message &req, google::protobuf::Message &resp, bool isFast = false) {
    Protocol::MySvrMessage mySvrReq;
    Protocol::MySvrMessage mySvrResp;
    createMySvrByPb(mySvrReq, req);
    if (isFast) {
      mySvrReq.EnableFastResp();  // 开启fast-resp模式
    }
    RpcCallRaw(mySvrReq, mySvrResp);
    if (mySvrResp.StatusCode() != 0) return mySvrResp.StatusCode();
    if (not Protocol::MixedCodec::PbParseFromMySvr(resp, mySvrResp)) {
      return PARSE_FAILED;
    }
    return mySvrResp.StatusCode();
  }
  void PushCallRaw(Protocol::MySvrMessage &mySvrMessage) {
    status_code_ = 0;
    message_ = "success";
    Common::TimeStat timeStat;
    Common::Defer defer([&mySvrMessage, &timeStat, this]() {
      // 写到socket就返回，则直接合入本地调用栈信息
      DistributedTrace::AddTraceInfo(mySvrMessage.context_.service_name(), mySvrMessage.context_.rpc_name(),
                                     timeStat.GetSpendTimeUs(), status_code_, message_);
    });
    std::string serviceName = mySvrMessage.context_.service_name();
    auto sockErrorDeal = [&mySvrMessage, this](int status_code, std::string desc) {
      status_code_ = status_code;
      message_ = strerror(errno);
      mySvrMessage.context_.set_status_code(status_code);
      CTX_ERROR(mySvrMessage.context_, "%s", desc.c_str());
    };
    Protocol::MySvrCodec codec;
    mySvrMessage.context_.set_parent_stack_id(ReqCtx.Get().current_stack_id());
    mySvrMessage.context_.set_stack_alloc_id(ReqCtx.Get().stack_alloc_id());
    if (not PushRetry(serviceName, codec, &mySvrMessage, sockErrorDeal)) {
      return;
    }
  }
  void RpcCallRaw(Protocol::MySvrMessage &req, Protocol::MySvrMessage &resp) {
    status_code_ = 0;
    message_ = "success";
    Common::TimeStat timeStat;
    Common::Defer defer([&req, &resp, &timeStat, this]() {
      if (0 == status_code_ && not MyCoroutine::CoroutineIsInBatch(SCHEDULE)) {
        DistributedTrace::MergeTraceInfo(resp.context_);  // 非batch且rpc调用成功，则直接合入rpc调用栈信息。
      } else {
        // batch或者rpc调用失败，则合入本地调用栈信息
        DistributedTrace::AddTraceInfo(req.context_.service_name(), req.context_.rpc_name(), timeStat.GetSpendTimeUs(),
                                       status_code_, message_);
      }
    });
    std::string serviceName = req.context_.service_name();
    auto errorDeal = [&req, &resp, this](int status_code, std::string desc) {
      status_code_ = status_code;
      message_ = strerror(errno);
      resp.context_.set_status_code(status_code);
      CTX_ERROR(req.context_, "%s", desc.c_str());
    };
    Protocol::MySvrCodec codec;
    Protocol::MySvrMessage *respMessage = nullptr;
    req.context_.set_parent_stack_id(ReqCtx.Get().current_stack_id());
    req.context_.set_stack_alloc_id(ReqCtx.Get().stack_alloc_id());
    if (not CallRetry(serviceName, codec, &req, (void **)&respMessage, nullptr, errorDeal)) {
      return;
    }
    resp.CopyFrom(*respMessage);
    delete respMessage;
  }

 private:
  void getServiceNameAndRpcName(google::protobuf::Message &req, std::string &serviceName, std::string &rpcName) {
    /* fullName，demo:
        MySvr.Echo.EchoMySelfRequest
        MySvr.Echo.OneWayMessage
     */
    std::string fullName = (std::string)(req.GetDescriptor()->full_name());
    std::vector<std::string> items;
    Common::Strings::Split(fullName, ".", items);
    serviceName = items[1];
    rpcName = items[2].substr(0, items[2].size() - 7);  // 去掉"Request"或者"Message"后缀长度
  }
  void createMySvrByPb(Protocol::MySvrMessage &mySvrMessage, google::protobuf::Message &pbMessage) {
    std::string rpcName;
    std::string serviceName;
    getServiceNameAndRpcName(pbMessage, serviceName, rpcName);
    mySvrMessage.context_.set_rpc_name(rpcName);
    mySvrMessage.context_.set_service_name(serviceName);
    mySvrMessage.context_.set_log_id(ReqCtx.Get().log_id());  // 传递分布式调用日志id
    Protocol::MixedCodec::PbSerializeToMySvr(pbMessage, mySvrMessage, 0);
  }

 private:
  int32_t status_code_{0};
  std::string message_{"success"};
};
}  // namespace Core
