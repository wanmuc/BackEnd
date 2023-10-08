#pragma once

#include <google/protobuf/util/json_util.h>
#include <json/json.h>

namespace Common {
class Convert {
 public:
  static bool Pb2JsonStr(const google::protobuf::Message &message, std::string &jsonStr, bool addWhitespace = false) {
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = addWhitespace;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    options.always_print_enums_as_ints = true;
    return google::protobuf::util::MessageToJsonString(message, &jsonStr, options).ok();
  }
  static bool JsonStr2Pb(const std::string &jsonStr, google::protobuf::Message &message) {
    return google::protobuf::util::JsonStringToMessage(jsonStr, &message).ok();
  }
  static bool Pb2Json(const google::protobuf::Message &message, Json::Value &value) {
    std::string jsonStr;
    if (not Pb2JsonStr(message, jsonStr)) return false;
    Json::Reader reader;
    return reader.parse(jsonStr, value);
  }
  static bool Json2Pb(Json::Value &value, google::protobuf::Message &message) {
    Json::FastWriter fastWriter;
    std::string jsonStr;
    jsonStr = fastWriter.write(value);
    return JsonStr2Pb(jsonStr, message);
  }
};  // namespace Convert
}  // namespace Common
