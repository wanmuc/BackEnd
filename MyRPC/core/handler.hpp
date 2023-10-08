#pragma once

#include "../common/log.hpp"
#include "../common/statuscode.hpp"
#include "../protocol/mixedcodec.hpp"
#include "coroutineio.hpp"
#include "coroutinelocal.hpp"
#include "distributedtrace.hpp"
#include "epollctl.hpp"

#define RPC_HANDLER(NAME, HANDLER, PB_REQ_TYPE, PB_RESP_TYPE, req, resp)                        \
  do {                                                                                          \
    Context ctx = req.context_;                                                                 \
    if (ctx.rpc_name() == NAME) {                                                               \
      PB_REQ_TYPE pbReq;                                                                        \
      PB_RESP_TYPE pbResp;                                                                      \
      bool convert = Protocol::MixedCodec::PbParseFromMySvr(pbReq, req);                        \
      int ret = 0;                                                                              \
      if (convert) {                                                                            \
        ret = HANDLER(pbReq, pbResp);                                                           \
      } else {                                                                                  \
        ret = PARSE_FAILED;                                                                     \
      }                                                                                         \
      Protocol::MixedCodec::PbSerializeToMySvr(pbResp, resp, ret);                              \
      std::string reqJson, respJson;                                                            \
      Common::Convert::Pb2JsonStr(pbReq, reqJson);                                              \
      Common::Convert::Pb2JsonStr(pbResp, respJson);                                            \
      CTX_TRACE(ctx, NAME " ret[%d],req[%s],resp[%s]", ret, reqJson.c_str(), respJson.c_str()); \
    }                                                                                           \
  } while (0)

extern Core::CoroutineLocal<Core::TimeOut> RpcTimeOut;
extern Core::CoroutineLocal<MySvr::Base::Context> ReqCtx;

namespace Core {
class MyHandler {
 public:
  void HandlerEntry(EventData *eventData) {
    auto releaseConn = [eventData](const std::string &error) {
      WARN("releaseConn %s, events=%s", error.c_str(), EpollCtl::EventReadable(eventData->events_).c_str());
      EpollCtl::ClearEvent(eventData->epoll_fd_, eventData->fd_);
      delete eventData;  // 释放内存
    };
    Common::TimeStat timeStat;
    Protocol::MixedCodec codec;
    void *req = nullptr;
    void *resp = nullptr;
    if (not readReqMessage(eventData, codec, &req, releaseConn)) {
      return;
    }
    auto codecType = codec.GetCodecType();
    Common::Defer defer([&req, &resp, codecType, this]() { release(req, resp, codecType); });
    Protocol::Packet pkt;
    resp = createResp(req, codecType);
    if (isFastResp(req, codecType)) {  // fast-resp模式先回包，再做业务处理
      setFastRespContext(req, resp, timeStat);
      codec.Encode(resp, pkt);
      EpollCtl::ModToWriteEvent(eventData->epoll_fd_, eventData->fd_, eventData);  // 监听可写事件
      if (not writeRespMessage(eventData, pkt, releaseConn)) {
        return;
      }
    }
    // 每个从协程都只能有一个IO事件唤醒点，在handler可能存在其他IO的唤醒点（调用其他rpc时），所以这里暂时清空对客户端IO事件的监听。
    EpollCtl::ClearEvent(eventData->epoll_fd_, eventData->fd_, false);
    handler(req, resp, codecType, timeStat);  // 业务处理，由具体的业务实现
    if (isReqResp(req, codecType)) {          // req-resp模式需要在handler之后再回包
      codec.Encode(resp, pkt);
      EpollCtl::AddWriteEvent(eventData->epoll_fd_, eventData->fd_, eventData);  // 监听可写事件
      if (not writeRespMessage(eventData, pkt, releaseConn)) {
        return;
      }
      // req-resp模式是从可写事件切换成可读事件的监听
      EpollCtl::ModToReadEvent(eventData->epoll_fd_, eventData->fd_, eventData);
    } else {
      // oneway和fast-resp模式是重启监听可读事件
      EpollCtl::AddReadEvent(eventData->epoll_fd_, eventData->fd_, eventData);
    }
    // 请求处理完，关联协程id设置无效的id，连接上后续请求才能再创建新的协程来处理
    eventData->cid_ = MyCoroutine::INVALID_ROUTINE_ID;
  }
  virtual void MySvrHandler(Protocol::MySvrMessage &req, Protocol::MySvrMessage &resp) = 0;

 private:
  bool readReqMessage(EventData *eventData, Protocol::MixedCodec &codec, void **req,
                      std::function<void(const std::string &error)> releaseConn) {
    RpcTimeOut.Set(TimeOut());  // 这里需要重新设置，因为在handler中可能存在rpc调用会覆盖超时配置
    while (true) {
      ssize_t ret = Core::CoRead(eventData->fd_, codec.Data(), codec.Len(), false);
      if (0 == ret) {
        releaseConn("peer close connection");
        return false;
      }
      if (ret < 0) {
        releaseConn(Common::Strings::StrFormat((char *)"read failed. errMsg[%s]", strerror(errno)));
        return false;
      }
      if (not codec.Decode(ret)) {
        releaseConn("decode failed.");
        return false;
      }
      *req = codec.GetMessage();
      if (*req) {
        return true;
      }
    }
  }
  bool writeRespMessage(EventData *eventData, Protocol::Packet &pkt,
                        std::function<void(const std::string &error)> releaseConn) {
    RpcTimeOut.Set(TimeOut());  // 这里需要重新设置，因为在handler中可能存在rpc调用会覆盖超时配置
    ssize_t sendLen = 0;
    uint8_t *buf = pkt.DataRaw();
    ssize_t needSendLen = pkt.UseLen();
    while (sendLen != needSendLen) {  // 写操作
      ssize_t ret = Core::CoWrite(eventData->fd_, buf + sendLen, needSendLen - sendLen, false);
      if (ret < 0) {
        releaseConn(Common::Strings::StrFormat((char *)"write failed. errMsg[%s]", strerror(errno)));
        return false;
      }
      sendLen += ret;
    }
    return true;
  }
  void handler(void *req, void *resp, Protocol::CodecType codecType, Common::TimeStat &timeStat) {
    if (Protocol::HTTP == codecType) {
      Protocol::HttpMessage *httpReq = (Protocol::HttpMessage *)req;
      Protocol::HttpMessage *httpResp = (Protocol::HttpMessage *)resp;
      if (not httpRequestValidCheck(httpReq, httpResp)) {
        return;
      }
      Protocol::MySvrMessage mySvrReq;
      Protocol::MySvrMessage mySvrResp;
      Protocol::MixedCodec::Http2MySvr(*httpReq, mySvrReq);
      DistributedTrace::InitTraceInfo(mySvrReq.context_);
      mySvrResp.head_.flag_ = mySvrReq.head_.flag_;
      MySvrHandler(mySvrReq, mySvrResp);  // 转换成MySvr协议的handler调用
      DistributedTrace::AddTraceInfo(timeStat.GetSpendTimeUs(), mySvrResp.StatusCode(), mySvrResp.Message());
      ReqCtx.Get().set_status_code(mySvrResp.StatusCode());
      mySvrResp.context_.CopyFrom(ReqCtx.Get());
      DistributedTrace::PrintTraceInfo(ReqCtx.Get(), ReqCtx.Get().current_stack_id(), 0);
      Protocol::MixedCodec::MySvr2Http(mySvrResp, *httpResp);
    } else {
      Protocol::MySvrMessage *mySvrReq = (Protocol::MySvrMessage *)req;
      Protocol::MySvrMessage *mySvrResp = (Protocol::MySvrMessage *)resp;
      if (not mySvrRequestValidCheck(mySvrReq, mySvrResp)) {
        return;
      }
      DistributedTrace::InitTraceInfo(mySvrReq->context_);
      mySvrResp->head_.flag_ = mySvrReq->head_.flag_;
      MySvrHandler(*mySvrReq, *mySvrResp);
      DistributedTrace::AddTraceInfo(timeStat.GetSpendTimeUs(), mySvrResp->StatusCode(), mySvrResp->Message());
      ReqCtx.Get().set_status_code(mySvrResp->StatusCode());
      mySvrResp->context_.CopyFrom(ReqCtx.Get());
      DistributedTrace::PrintTraceInfo(ReqCtx.Get(), ReqCtx.Get().current_stack_id(), 0);
    }
  }
  virtual bool isSupportRpc(std::string serviceName, std::string rpcName) {
    if (service_name_ != serviceName) return false;
    if (rpc_names_.find(rpcName) == rpc_names_.end()) return false;
    return true;
  }
  bool mySvrRequestValidCheck(Protocol::MySvrMessage *request, Protocol::MySvrMessage *response) {
    if (not isSupportRpc(request->context_.service_name(), request->context_.rpc_name())) {
      response->context_.set_status_code(NOT_SUPPORT_RPC);
      return false;
    }
    return true;
  }
  bool httpRequestValidCheck(Protocol::HttpMessage *request, Protocol::HttpMessage *response) {
    std::string contentType = request->GetHeader("Content-Type");
    if (contentType == "" || contentType.find("application/json") == std::string::npos) {  // body必须是json格式
      response->SetBody(R"({"message": "Content-Type not json"})");
      response->SetStatusCode(Protocol::BAD_REQUEST);
      return false;
    }
    std::string rpcName = request->GetHeader("rpc_name");
    std::string serviceName = request->GetHeader("service_name");
    if (not isSupportRpc(serviceName, rpcName)) {
      response->SetBody(R"({"message":"rpc_name or service_name not support"})");
      response->SetStatusCode(Protocol::BAD_REQUEST);
      return false;
    }
    std::string method, url;
    request->GetMethodAndUrl(method, url);
    if (method != "POST") {  // 只支持post请求
      response->SetBody(R"({"message":"not post request"})");
      response->SetStatusCode(Protocol::BAD_REQUEST);
      return false;
    }
    if (url != "/index") {  // url只支持/index
      response->SetBody(R"({"message":"url is invalid"})");
      response->SetStatusCode(Protocol::BAD_REQUEST);
      return false;
    }
    return true;
  }
  void *createResp(void *req, Protocol::CodecType codecType) {
    if (Protocol::HTTP == codecType) return new Protocol::HttpMessage;
    Protocol::MySvrMessage *mySvrReq = (Protocol::MySvrMessage *)req;
    Protocol::MySvrMessage *mySvrResp = new Protocol::MySvrMessage;
    if (isFastResp(req, codecType)) {  // fast-resp模式，body则使用默认的MySvr::Base::FastRespResponse类对象来设置
      mySvrResp->head_.flag_ = mySvrReq->head_.flag_;
      auto fastRespResponse = MySvr::Base::FastRespResponse();
      Protocol::MixedCodec::PbSerializeToMySvr(fastRespResponse, *mySvrResp, 0);
    }
    return mySvrResp;
  }
  void release(void *req, void *resp, Protocol::CodecType codecType) {
    if (Protocol::HTTP == codecType) {
      delete (Protocol::HttpMessage *)req;
      delete (Protocol::HttpMessage *)resp;
    } else {
      delete (Protocol::MySvrMessage *)req;
      delete (Protocol::MySvrMessage *)resp;
    }
  }
  bool isOneway(void *req, Protocol::CodecType codecType) {
    if (Protocol::HTTP == codecType) return false;  // http协议不支持oneway
    Protocol::MySvrMessage *mySvrMessage = (Protocol::MySvrMessage *)req;
    return mySvrMessage->IsOneway();
  }
  bool isFastResp(void *req, Protocol::CodecType codecType) {
    if (Protocol::HTTP == codecType) return false;  // http协议不支持fast-resp
    Protocol::MySvrMessage *mySvrMessage = (Protocol::MySvrMessage *)req;
    return mySvrMessage->IsFastResp();
  }
  bool isReqResp(void *req, Protocol::CodecType codecType) {
    if (Protocol::HTTP == codecType) return true;  // http协议只支持fast-resp
    return not isOneway(req, codecType) && not isFastResp(req, codecType);
  }
  void setFastRespContext(void *req, void *resp, Common::TimeStat &timeStat) {
    Protocol::MySvrMessage *mySvrReq = (Protocol::MySvrMessage *)req;
    Protocol::MySvrMessage *mySvrResp = (Protocol::MySvrMessage *)resp;
    DistributedTrace::InitTraceInfo(mySvrReq->context_);
    DistributedTrace::AddTraceInfo(timeStat.GetSpendTimeUs(), 0, "success");
    mySvrResp->context_.CopyFrom(ReqCtx.Get());
    DistributedTrace::PrintTraceInfo(ReqCtx.Get(), ReqCtx.Get().current_stack_id(), 0);
  }

 protected:
  std::string service_name_;
  std::unordered_set<std::string> rpc_names_;
};
}  // namespace Core
