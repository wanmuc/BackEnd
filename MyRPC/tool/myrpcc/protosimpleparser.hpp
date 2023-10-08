#pragma once

#include <assert.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../../common/strings.hpp"

#define GREEN_BEGIN "\033[32m"
#define RED_BEGIN "\033[31m"
#define COLOR_END "\033[0m"

using namespace std;

enum ParseStatus {
  INIT = 0,           // 初始化状态
  IDENTIFIER = 1,     // 标识符（包括关键字）-> syntax "proto3" package MySvr.Auth message GenTicketRequest
  SPECIAL_CHAR = 2,   // 特殊字符 -> = ; { } ( ) [ ]
  ORDER_NUM = 3,      // 序号 -> 1 2 3
  COMMENT_BEGIN = 4,  // 注释模式
  COMMENT_STAR = 5,   // 注释模式1 -> /*
  COMMENT_DOUBLE_SLASH = 6,  // 注释模式2 -> //
};

enum RpcMode {
  REQ_RESP = 1,   // Request-Response
  ONE_WAY = 2,    // Oneway
  FAST_RESP = 3,  // Fast-Response
};

typedef struct RpcInfo {
  string rpc_name_;
  RpcMode rpc_mode_;
  string request_name_;
  string response_name_;
  string cpp_file_name_;
} RpcInfo;

typedef struct ServiceInfo {
  std::string package_name_;
  std::string service_name_;
  std::string cpp_namespace_name_;
  std::string handler_file_prefix_;
  std::string port_;
  std::vector<RpcInfo> rpc_infos_;
} ServiceInfo;

class ProtoSimpleParser {
 public:
  static bool Parse2Token(string proto, vector<string>& tokens) {
    if (proto == "") return false;
    ifstream in;
    string line;
    in.open(proto.c_str());
    if (not in.is_open()) return false;
    std::string token = "";
    ParseStatus parseStatus = INIT;
    while (getline(in, line)) {
      parseLine(line, token, parseStatus, tokens);
    }
    return true;
  }
  static void Parse2ServiceInfo(vector<string>& tokens, ServiceInfo& serviceInfo) {
    for (size_t i = 0; i < tokens.size(); i++) {
      // 提取package关键字后的包名
      if (tokens[i] == "package" && i + 1 < tokens.size()) {
        serviceInfo.package_name_ = tokens[i + 1];
      }
      // 提取service关键字后的服务名称
      if (tokens[i] == "service" && i + 1 < tokens.size()) {
        serviceInfo.service_name_ = tokens[i + 1];
      }
      // demo: option (MySvr.Base.Port) = 1693;
      // 提取服务监听的端口号
      if (tokens[i] == "option" && i + 5 < tokens.size() && tokens[i + 2] == "MySvr.Base.Port") {
        serviceInfo.port_ = tokens[i + 5];
      }
      /* demo:
           rpc OneWay(OneWayMessage) returns(MySvr.Base.OneWayResponse) {
              option (MySvr.Base.MethodMode) = 2;
           };
       */
      if (tokens[i] == "rpc" && i + 7 < tokens.size()) {
        RpcInfo rpcInfo;
        rpcInfo.rpc_mode_ = REQ_RESP;  // 默认是REQ_RESP模式
        rpcInfo.rpc_name_ = tokens[i + 1];
        rpcInfo.request_name_ = tokens[i + 3];
        rpcInfo.response_name_ = tokens[i + 7];
        rpcInfo.cpp_file_name_ = rpcInfo.rpc_name_ + ".cpp";
        if (i + 15 < tokens.size() && tokens[i + 12] == "MySvr.Base.MethodMode") {
          if (tokens[i + 15] == "2") {
            rpcInfo.rpc_mode_ = ONE_WAY;
          }
          if (tokens[i + 15] == "3") {
            rpcInfo.rpc_mode_ = FAST_RESP;
          }
        }
        serviceInfo.rpc_infos_.push_back(rpcInfo);
      }
    }
    preDealServiceInfo(serviceInfo);
  }

 private:
  static void preDealServiceInfo(ServiceInfo& serviceInfo) {
    vector<string> items;
    Common::Strings::Split(serviceInfo.package_name_, ".", items);
    if (items.size() != 2) {
      cout << RED_BEGIN << "package_name only support secondary package." << COLOR_END << endl;
      exit(-1);
    }
    if (items[1] != serviceInfo.service_name_) {
      cout << RED_BEGIN << "package_name's suffix[" << items[1] << "] and service_name[" << serviceInfo.service_name_
           << "] not match." << COLOR_END << endl;
      exit(-1);
    }
    serviceInfo.cpp_namespace_name_ = Common::Strings::Join(items, "::");
    serviceInfo.handler_file_prefix_ = serviceInfo.service_name_;
    Common::Strings::ToLower(serviceInfo.handler_file_prefix_);
    for (auto& rpcInfo : serviceInfo.rpc_infos_) {
      Common::Strings::ToLower(rpcInfo.cpp_file_name_);
      size_t len = rpcInfo.request_name_.length();
      if (len <= 7) {
        cout << RED_BEGIN << "request_name[" << rpcInfo.request_name_ << "] is too short" << COLOR_END << endl;
        exit(-1);
      }
      string suffix = rpcInfo.request_name_.substr(len - 7);
      if (not(suffix != "Request" || suffix != "Message")) {
        cout << RED_BEGIN << "request_name[" << rpcInfo.request_name_ << "] is not end with Request or Message"
             << COLOR_END << endl;
        exit(-1);
      }
      if (suffix == "Request" && rpcInfo.rpc_name_ + "Request" != rpcInfo.request_name_) {
        cout << RED_BEGIN << "rpc_name[" << rpcInfo.rpc_name_ << "] is not request_name[" << rpcInfo.request_name_
             << "] prefix" << COLOR_END << endl;
        exit(-1);
      }
      if (suffix == "Message" && rpcInfo.rpc_name_ + "Message" != rpcInfo.request_name_) {
        cout << RED_BEGIN << "rpc_name[" << rpcInfo.rpc_name_ << "] is not request_name[" << rpcInfo.request_name_
             << "] prefix" << COLOR_END << endl;
        exit(-1);
      }
      if (rpcInfo.rpc_mode_ == ONE_WAY && rpcInfo.response_name_ != "MySvr.Base.OneWayResponse") {
        cout << RED_BEGIN << "oneway rpc response must MySvr.Base.OneWayResponse" << COLOR_END << endl;
        exit(-1);
      }
      if (rpcInfo.rpc_mode_ == FAST_RESP && rpcInfo.response_name_ != "MySvr.Base.FastRespResponse") {
        cout << RED_BEGIN << "fast response rpc response must MySvr.Base.FastRespResponse" << COLOR_END << endl;
        exit(-1);
      }
      vector<string> items;
      Common::Strings::Split(rpcInfo.request_name_, ".", items);
      rpcInfo.request_name_ = items[items.size() - 1];
      items.clear();
      Common::Strings::Split(rpcInfo.response_name_, ".", items);
      rpcInfo.response_name_ = items[items.size() - 1];
    }
  }
  static bool isSpecial(char c) {
    return c == '=' || c == ';' || c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']';
  }
  static void parseLine(string line, std::string& token, ParseStatus& parseStatus, vector<string>& tokens) {
    auto getOneToken = [&token, &tokens, &parseStatus](ParseStatus newStatus) {
      if (token != "") tokens.push_back(token);
      token = "";
      parseStatus = newStatus;
    };
    for (size_t i = 0; i < line.size(); i++) {
      char c = line[i];
      if (parseStatus == INIT) {
        if (isblank(c)) continue;
        if (isdigit(c)) {  // 数字
          token = c;
          parseStatus = ORDER_NUM;
        } else if (c == '/') {  // 注释
          if (i + 1 < line.size() && (line[i + 1] == '*' || line[i + 1] == '/')) {
            parseStatus = COMMENT_BEGIN;
          } else {
            token = c;
            parseStatus = IDENTIFIER;
          }
        } else if (isSpecial(c)) {  // 特殊字符
          token = c;
          parseStatus = SPECIAL_CHAR;
        } else {  // 执行到这里就是标识符
          token = c;
          parseStatus = IDENTIFIER;
        }
      } else if (parseStatus == IDENTIFIER) {
        if (isblank(c)) {
          getOneToken(INIT);
        } else if (isdigit(c)) {
          token += c;
        } else if (c == '/') {
          if (i + 1 < line.size() && (line[i + 1] == '*' || line[i + 1] == '/')) {
            getOneToken(COMMENT_BEGIN);
          } else {
            token += c;
          }
        } else if (isSpecial(c)) {
          getOneToken(SPECIAL_CHAR);
          token = c;
        } else {
          token += c;
        }
      } else if (parseStatus == SPECIAL_CHAR) {
        if (isblank(c)) {
          getOneToken(INIT);
        } else if (isdigit(c)) {
          getOneToken(ORDER_NUM);
          token = c;
        } else if (c == '/') {
          if (i + 1 < line.size() && (line[i + 1] == '*' || line[i + 1] == '/')) {
            getOneToken(COMMENT_BEGIN);
          } else {
            getOneToken(IDENTIFIER);
            token = c;
          }
        } else if (isSpecial(c)) {
          getOneToken(SPECIAL_CHAR);
          token = c;
        } else {
          getOneToken(IDENTIFIER);
          token = c;
        }
      } else if (parseStatus == ORDER_NUM) {
        if (isblank(c)) {
          getOneToken(INIT);
        } else if (isdigit(c)) {
          token += c;
        } else if (c == '/') {
          if (i + 1 < line.size() && (line[i + 1] == '*' || line[i + 1] == '/')) {
            getOneToken(COMMENT_BEGIN);
          } else {
            getOneToken(IDENTIFIER);
            token = c;
          }
        } else if (isSpecial(c)) {
          getOneToken(SPECIAL_CHAR);
          token = c;
        } else {
          getOneToken(IDENTIFIER);
          token = c;
        }
      } else if (parseStatus == COMMENT_BEGIN) {
        if (c == '*') {
          parseStatus = COMMENT_STAR;
        } else if (c == '/') {
          parseStatus = COMMENT_DOUBLE_SLASH;
        } else {
          assert(0);
        }
      } else if (parseStatus == COMMENT_STAR) {
        if (c == '*' && i + 1 < line.size() && line[i + 1] == '/') {  // 尝试看下一个字符
          i++;                                                        // 跳过'/'
          parseStatus = INIT;                                         // 解析状态变成初始化
        }
      } else if (parseStatus == COMMENT_DOUBLE_SLASH) {
        if (i == line.size() - 1) {  // 双斜线的注释范围到行尾结束
          parseStatus = INIT;        // 解析状态变成初始化
        }
      } else {
        assert(0);
      }
    }
  }
};
