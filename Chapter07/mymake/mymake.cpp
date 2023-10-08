#include <fstream>
#include <iostream>

#include "makefileparser.h"
#include "makefiletarget.h"

using namespace std;

bool GetMakeFileName(string &makefile_name) {
  ifstream in;
  in.open("./makefile");  // 优先判定makefile文件
  if (in.is_open()) {
    makefile_name = "./makefile";
    return true;
  }
  in.open("./Makefile");
  if (in.is_open()) {
    makefile_name = "./Makefile";
    return true;
  }
  return false;
}

int main(int argc, char *argv[]) {
  MyMake::Parser parser;
  vector<vector<MyMake::Token>> tokens_list;
  string makefile_name;
  // 判断makefile或者Makefile是否存在
  if (not GetMakeFileName(makefile_name)) {
    cout << "mymake: *** No targets specified and no makefile found.  Stop." << endl;
    return -1;
  }
  // 词法分析
  if (not parser.ParseToToken(makefile_name, tokens_list)) {
    cout << "ParseToToken failed" << endl;
    return -1;
  }
  // 语法分析，生成编译依赖树（多叉树），并完成简单的语法校验
  map<string, MyMake::Target *> name_2_target;
  MyMake::Target *build_target = MyMake::Target::GenTarget(name_2_target, tokens_list);
  if (argc >= 2) {
    string build_name = string(argv[1]);
    if (name_2_target.find(build_name) == name_2_target.end()) {
      cout << "mymake: *** No rule to make target `" << build_name << "'.  Stop." << endl;
      exit(-1);
    }
    build_target = name_2_target[build_name];
  }
  // 执行编译操作，深度优先遍历多叉树，编译过程需要判断目标是否需要重新构建
  if (build_target->IsNeedReBuild()) {
    return MyMake::Target::BuildTarget(build_target);  // 一个深度优先遍历的构建过程
  }
  cout << "mymake: `" << build_target->Name() << "' is up to date." << endl;
  return 0;
}