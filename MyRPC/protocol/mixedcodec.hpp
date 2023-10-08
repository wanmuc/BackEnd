#pragma once

#include "../common/convert.hpp"
#include "httpcodec.hpp"
#include "mysvrcodec.hpp"

namespace Protocol {
class MixedCodec {
 public:
  ~MixedCodec() {
    if (codec_) delete codec_;
  }
  CodecType GetCodecType() {
    if (nullptr == codec_) return UNKNOWN;
    return codec_->Type();
  }
  uint8_t *Data() {
    if (nullptr == codec_) return &first_byte_;  // 无法确定具体协议之前，只读取一个字节
    return codec_->Data();
  }
  size_t Len() {
    if (nullptr == codec_) return 1;  // 无法确定具体协议之前，只读取一个字节
    return codec_->Len();
  }
  void *GetMessage() {
    if (nullptr == codec_) return nullptr;
    return codec_->GetMessage();
  }
  bool Encode(void *msg, Packet &pkt) {
    if (nullptr == codec_) return false;
    return codec_->Encode(msg, pkt);
  }
  bool Decode(size_t len) {
    assert(len >= 1);
    createCodec();
    assert(codec_ != nullptr);
    return codec_->Decode(len);
  }
  static void Http2MySvr(HttpMessage &httpMessage, MySvrMessage &mySvrMessage) {
    mySvrMessage.context_.set_service_name(httpMessage.GetHeader("service_name"));
    mySvrMessage.context_.set_rpc_name(httpMessage.GetHeader("rpc_name"));
    mySvrMessage.BodyEnableJson();  // body的格式设置为json
    size_t bodyLen = httpMessage.body_.size();
    mySvrMessage.body_.Alloc(bodyLen);
    memmove(mySvrMessage.body_.Data(), httpMessage.body_.data(), bodyLen);
    mySvrMessage.body_.UpdateUseLen(bodyLen);
  }
  static void MySvr2Http(MySvrMessage &mySvrMessage, HttpMessage &httpMessage) {
    httpMessage.SetStatusCode(OK);
    httpMessage.SetHeader("log_id", mySvrMessage.context_.log_id());
    httpMessage.SetHeader("status_code", std::to_string(mySvrMessage.context_.status_code()));
    if (0 == mySvrMessage.context_.status_code()) {
      assert(mySvrMessage.BodyIsJson());  // body此时必须是json str
      size_t len = mySvrMessage.body_.UseLen();
      httpMessage.SetBody(std::string((char *)mySvrMessage.body_.DataRaw(), len));
    } else {
      httpMessage.SetBody(R"({"message":")" + mySvrMessage.Message() + R"("})");
    }
  }
  static bool PbParseFromMySvr(google::protobuf::Message &pb, MySvrMessage &mySvr) {
    std::string str((char *)mySvr.body_.DataRaw(), mySvr.body_.UseLen());
    if (mySvr.BodyIsJson()) {  // json格式
      return Common::Convert::JsonStr2Pb(str, pb);
    }
    return pb.ParseFromString(str);
  }
  static void PbSerializeToMySvr(google::protobuf::Message &pb, MySvrMessage &mySvr, int statusCode) {
    std::string str;
    bool result = true;
    if (mySvr.BodyIsJson()) {  // json格式
      result = Common::Convert::Pb2JsonStr(pb, str);
    } else {
      result = pb.SerializeToString(&str);
    }
    if (not result) {
      mySvr.context_.set_status_code(SERIALIZE_FAILED);
      return;
    }
    mySvr.body_.Alloc(str.size());
    memmove(mySvr.body_.Data(), str.data(), str.size());
    mySvr.body_.UpdateUseLen(str.size());
    mySvr.context_.set_status_code(statusCode);
  }
  static void JsonStrSerializeToMySvr(std::string serviceName, std::string rpcName, std::string &jsonStr,
                                      MySvrMessage &mySvr) {
    mySvr.BodyEnableJson();
    mySvr.context_.set_service_name(serviceName);
    mySvr.context_.set_rpc_name(rpcName);
    mySvr.body_.Alloc(jsonStr.size());
    memmove(mySvr.body_.Data(), jsonStr.data(), jsonStr.size());
    mySvr.body_.UpdateUseLen(jsonStr.size());
  }

 private:
  void createCodec() {
    if (codec_ != nullptr) return;
    if (PROTO_MAGIC_AND_VERSION == first_byte_) {
      codec_ = new MySvrCodec;
    } else {
      codec_ = new HttpCodec;
    }
    memmove(codec_->Data(), &first_byte_, 1);  // 拷贝1个字节的内容
    first_byte_ = 0;
  }

 private:
  Codec *codec_{nullptr};
  uint8_t first_byte_{0};  // 第一个字节，用于判断具体的协议
};
}  // namespace Protocol