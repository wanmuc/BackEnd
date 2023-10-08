#pragma once

#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

class GenBase {
 public:
  static bool GenFile(string file, string content) {
    ofstream out;
    out.open(file);
    if (not out.is_open()) {
      cout << "open file[" << file << "] failed." << endl;
      return false;
    }
    out << content << endl;
    return true;
  }
  static bool ExecCmd(string cmd) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("call fork failed.");
      return false;
    }
    if (0 == pid) {
      cout << "exec cmd[" << cmd << "]" << endl;
      execl("/bin/bash", "bash", "-c", cmd.c_str(), nullptr);
      exit(1);
    }
    int status = 0;
    int ret = waitpid(pid, &status, 0);  // 父进程调用waitpid等待子进程执行子命令结束，并获取子命令的执行结果
    if (ret != pid) {
      perror("call waitpid failed.");
      return false;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      return true;
    }
    return false;
  }
  static bool IsFileExist(string file) { return ifstream(file).good(); }
  static bool Mkdir(string dir) {
    if (0 == mkdir(dir.c_str(), 0)) return true;
    if (errno == EEXIST) return true;
    return false;
  }
};