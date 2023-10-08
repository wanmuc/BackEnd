#pragma once
#include "genbase.hpp"
#include "protosimpleparser.hpp"

class GenMain : public GenBase {
 public:
  static bool Gen(ServiceInfo &serviceInfo) {
    string prefix = serviceInfo.handler_file_prefix_;
    if (IsFileExist(prefix + ".cpp")) {
      return true;
    }
    string serviceName = serviceInfo.service_name_;
    stringstream out;
    out << R"(#include "../../core/service.h")" << endl;
    out << R"(#include ")" + prefix + R"(handler.h")" << endl;
    out << endl;
    out << R"(int main(int argc, char *argv[]) {
  )" + serviceName +
               R"(Handler handler;
  SERVICE.Init(argc, argv);
  SERVICE.RegHandler(&handler);
  SERVICE.Run();
  return 0;
})";
    return GenFile(prefix + ".cpp", out.str());
  }
};