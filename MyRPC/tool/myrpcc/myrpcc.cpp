#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "../../common/cmdline.h"
#include "../../common/strings.hpp"
#include "genbuild.hpp"
#include "genconf.hpp"
#include "genhandler.hpp"
#include "geninstall.hpp"
#include "genmain.hpp"
#include "genmakefile.hpp"
#include "protosimpleparser.hpp"

#define GREEN_BEGIN "\033[32m"
#define RED_BEGIN "\033[31m"
#define COLOR_END "\033[0m"

using namespace std;

string proto;  // proto的定义文件
bool debug;    // 是否输出debug信息

void usage() {
  cout << "myrpcc -proto echo.proto [-debug]" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help     print usage" << endl;
  cout << "    -proto        protobuf file" << endl;
  cout << "    -debug        print debug info" << endl;
  cout << endl;
}

bool IsValidProto() {
  ifstream in;
  string line;
  string cmd = "protoc -I./proto -I../../protocol --cpp_out=./proto " + proto;
  string protoFile = "./proto/" + proto;
  in.open(protoFile.c_str());
  if (not in.is_open()) {
    cout << "open file[" << protoFile << "] failed." << strerror(errno) << endl;
    return false;
  }
  return GenBase::ExecCmd(cmd);
}

void printTokens(vector<string>& tokens) {
  for (auto token : tokens) {
    cout << token << endl;
  }
}

void printServiceInfo(ServiceInfo& serviceInfo) {
  cout << "package=" << serviceInfo.package_name_ << endl;
  cout << "service=" << serviceInfo.service_name_ << endl;
  cout << "port=" << serviceInfo.port_ << endl;
  cout << "namespace=" << serviceInfo.cpp_namespace_name_ << endl;
  cout << "handler_file_prefix=" << serviceInfo.handler_file_prefix_ << endl;
  for (auto rpcInfo : serviceInfo.rpc_infos_) {
    cout << "rpcInfo[name=" << rpcInfo.rpc_name_ << ",mode=" << rpcInfo.rpc_mode_ << ",req=" << rpcInfo.request_name_
         << ",resp=" << rpcInfo.response_name_ << ",rpc_file=" << rpcInfo.cpp_file_name_ << "]" << endl;
  }
}

bool genMySvrFiles(ServiceInfo& serviceInfo) {
  if (not GenMakefile::Gen()) return false;
  if (not GenInstall::Gen()) return false;
  if (not GenBuild::Gen(serviceInfo)) return false;
  if (not GenMain::Gen(serviceInfo)) return false;
  if (not GenHandler::Gen(serviceInfo)) return false;
  if (not GenConf::Gen(serviceInfo)) return false;
  if (not GenBase::ExecCmd("chmod +x ./build.sh ./install.sh")) return false;
  return true;
}

int main(int argc, char* argv[]) {
  Common::CmdLine::StrOptRequired(&proto, "proto");
  Common::CmdLine::BoolOpt(&debug, "debug");
  Common::CmdLine::SetUsage(usage);
  Common::CmdLine::Parse(argc, argv);
  if (not IsValidProto()) {
    cout << RED_BEGIN << proto << " invalid, please check proto file." << COLOR_END << endl;
    return -1;
  }
  vector<string> tokens;
  if (not ProtoSimpleParser::Parse2Token("./proto/" + proto, tokens)) {
    cout << RED_BEGIN << "proto parse failed, please check proto file." << COLOR_END << endl;
    return -1;
  }
  if (debug) {
    printTokens(tokens);
  }
  ServiceInfo serviceInfo;
  ProtoSimpleParser::Parse2ServiceInfo(tokens, serviceInfo);
  printServiceInfo(serviceInfo);
  cout << GREEN_BEGIN << "parse Proto[" << proto << "] success" << COLOR_END << endl;
  if (not genMySvrFiles(serviceInfo)) {
    cout << RED_BEGIN << "gen myrpc files failed." << COLOR_END << endl;
    return -1;
  }
  cout << GREEN_BEGIN << "gen myrpc files success" << COLOR_END << endl;
  return 0;
}