#pragma once
#include "genbase.hpp"

class GenInstall : public GenBase {
 public:
  static bool Gen() {
    if (IsFileExist("install.sh")) {
      return true;
    }
    string content = R"(#!/bin/bash
PROG=$(pwd | xargs basename)
mkdir -p /home/backend/log/${PROG} # 创建日志文件目录
mkdir -p /home/backend/service/${PROG} # 创建可执行文件和配置文件所在目录
mkdir -p /home/backend/script # 创建执行脚本的目录
mkdir -p /home/backend/route # 创建路由文件的目录
cp -f ./${PROG} /home/backend/service/${PROG} # 拷贝可执行文件
cp -f ./conf/${PROG}.conf /home/backend/service/${PROG} # 拷贝配置文件
cp -f ./conf/${PROG}_client.conf /home/backend/route # 发布路由文件
cp -f ../daemond.sh /home/backend/script/${PROG} # 拷贝启停脚本
chmod +x /home/backend/script/${PROG}
chmod +x /home/backend/service/${PROG}/${PROG})";
    return GenFile("install.sh", content);
  }
};
