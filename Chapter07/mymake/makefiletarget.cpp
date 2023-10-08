#include "makefiletarget.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace MyMake {
bool Target::IsNeedReBuild() {
  if (not is_real_) {  // 非真实的目标不用重建
    return false;
  }
  if (name_ == ".PHONY") {  // 伪目标声明的关键字，则需要重新构建
    return true;
  }
  if (relate_cmds_.size() <= 0) {  // 真实目前且不是.PHONY，没有关联命令的目标，直接报错
    std::cout << "mymake: Nothing to be done for `" << name_ << "'." << std::endl;
    exit(4);
  }
  if (relate_targets_.size() <= 0) {  // 依赖的目标为空，则需要重新构建，例如：clean
    return true;
  }
  last_update_time_ = GetLastUpdateTime();
  for (size_t i = 0; i < relate_targets_.size(); i++) {
    relate_targets_[i]->last_update_time_ = relate_targets_[i]->GetLastUpdateTime();
    // 文件不存在（last_update_time_值为-1）或者依赖的目标已经更新了
    if (relate_targets_[i]->last_update_time_ == -1 || relate_targets_[i]->last_update_time_ > last_update_time_) {
      return true;
    }
    if (relate_targets_[i]->IsNeedReBuild()) {  // 需要再检查依赖的目标是否需要重新构建
      return true;
    }
  }
  return false;  // 所有依赖目标的更新时间都小于等于当前的目标的更新时间，则当前目标不用重建
}

int Target::ExecRelateCmd() {
  if (not IsNeedReBuild()) {  // 不用构建，则直接
    return 0;
  }
  int result = 0;
  for (auto& cmd : relate_cmds_) {
    result = execCmd(cmd);
    if (result) {
      std::cout << "mymake: *** [" << name_ << "] Error " << result << std::endl;
      exit(3);
    }
  }
  return 0;
}

int64_t Target::GetLastUpdateTime() {
  struct stat file_stat;
  std::string target_name = "./" + name_;
  if (stat(target_name.c_str(), &file_stat) == -1) {
    return -1;
  }
  return file_stat.st_mtime;
}

int Target::execCmd(std::string cmd) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("call fork failed.");
    return -1;
  }
  if (0 == pid) {
    std::cout << cmd << std::endl;
    execl("/bin/bash", "bash", "-c", cmd.c_str(), nullptr);
    exit(1);
  }
  int status = 0;
  int ret = waitpid(pid, &status, 0);  // 父进程调用waitpid等待子进程执行子命令结束，并获取子命令的执行结果
  if (ret != pid) {
    perror("call waitpid failed.");
    return -1;
  }
  if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
    return 0;
  }
  return WEXITSTATUS(status);
}

Target* Target::QueryOrCreateTarget(std::map<std::string, Target*>& name_2_target, std::string name) {
  if (name_2_target.find(name) == name_2_target.end()) {
    name_2_target[name] = new Target(name);
  }
  return name_2_target[name];
}

Target* Target::GenTarget(std::map<std::string, Target*>& name_2_target, std::vector<std::vector<Token>>& tokens_list) {
  Target* root = nullptr;
  Target* target = nullptr;
  for (auto& tokens : tokens_list) {
    if (tokens[0].token_type == IDENTIFIER) {  // 一个目标
      if (tokens.size() <= 1 || tokens[1].token_type != COLON) {  // 目标之后必须是冒号
        // 语法错误
        std::cout << "makefile:" << tokens[0].line_number << ": *** missing separator.  Stop." << std::endl;
        exit(1);
      }
      target = QueryOrCreateTarget(name_2_target, tokens[0].text);
      for (size_t i = 2; i < tokens.size(); i++) {  // 创建依赖的target
        Target* relate_target = QueryOrCreateTarget(name_2_target, tokens[i].text);
        target->SetRelateTarget(relate_target);
      }
      target->SetIsReal(true);
      if (nullptr == root) {
        root = target;  // 第一个target就是多叉树根节点
      }
      continue;
    }
    if (tokens[0].token_type == TAB) {  // 一条命令
      if (tokens[0].line_pos != 0) {  // tab键必须在开头位置
        std::cout << "makefile:" << tokens[0].line_number << ": *** tab must at line begin.  Stop." << std::endl;
        exit(3);
      }
      if (tokens.size() == 1) {  // 一条空命令，直接过滤
        continue;
      }
      assert(tokens.size() == 2);
      if (target == nullptr) {
        std::cout << "makefile:" << tokens[0].line_number << ": *** recipe commences before target.  Stop."
                  << std::endl;
        exit(2);
      }
      assert(tokens[1].token_type == CMD);
      target->SetRelateCmd(tokens[1].text);
      continue;
    }
    // 语法错误
    std::cout << "makefile:" << tokens[0].line_number << ": *** missing separator.  Stop." << std::endl;
    exit(1);
  }
  return root;
}

int Target::BuildTarget(Target* target) {
  if (target->relate_targets_.size() <= 0) {  // 叶子节点，直接执行自身关联的命令，并返回，例如mymake.cpp这个目标
    return target->ExecRelateCmd();
  }
  for (auto& relate_target : target->relate_targets_) {  // 执行依赖的目标的构建
    BuildTarget(relate_target);
  }
  return target->ExecRelateCmd();  // 构建完依赖的目标，执行对目标自身构建的命令
}
}  // namespace MyMake
