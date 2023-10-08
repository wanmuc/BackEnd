#pragma once

#include <map>
#include <string>
#include <vector>

#include "makefileparser.h"

namespace MyMake {
class Target {
 public:
  Target(std::string name) : name_(name), is_real_(false) {}
  void SetIsReal(bool is_real) { is_real_ = is_real; }
  void SetRelateCmd(std::string relate_cmd) { relate_cmds_.push_back(relate_cmd); }
  void SetLastUpdateTime(int64_t last_update_time) { last_update_time_ = last_update_time; }
  void SetRelateTarget(Target* target) { relate_targets_.push_back(target); }
  bool IsNeedReBuild();
  int ExecRelateCmd();
  int64_t GetLastUpdateTime();
  std::string Name() { return name_; }
  static Target* QueryOrCreateTarget(std::map<std::string, Target*>& name_2_target, std::string name);
  static Target* GenTarget(std::map<std::string, Target*>& name_2_target, std::vector<std::vector<Token>>& tokens_list);
  static int BuildTarget(Target* target);

 private:
  int execCmd(std::string cmd);

 private:
  std::string name_;  // 目标名称
  bool is_real_;  // 是否位真正的目标，例如：mymake.o : mymake.cpp 中，mymake.o是真正的目标，mymake.cpp是一个虚拟的目标
  int64_t last_update_time_;              // 目标最后更新的时间
  std::vector<std::string> relate_cmds_;  // 目标关联的命令
  std::vector<Target*> relate_targets_;   // 依赖的目标列表
};
}  // namespace MyMake