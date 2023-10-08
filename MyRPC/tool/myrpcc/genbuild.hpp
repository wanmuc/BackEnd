#pragma once
#include "genbase.hpp"
#include "protosimpleparser.hpp"

extern string proto;

class GenBuild : public GenBase {
 public:
  static bool Gen(ServiceInfo &serviceInfo) {
    string content = R"(#!/bin/bash
protoc -I./proto -I../../protocol --cpp_out=./proto )" +
                     proto + R"(
make clean
make -j$(nproc))";
    return GenFile("build.sh", content);
  }
};