#pragma once
#include <assert.h>

#include <unordered_map>

namespace Common {
class Argv {
 public:
  Argv& Set(std::string name, void* arg) {
    argv_[name] = arg;
    return *this;
  }
  template <class Type>
  Type& Arg(std::string name) {
    auto iter = argv_.find(name);
    assert(iter != argv_.end());
    return *(Type*)iter->second;
  }

 private:
  std::unordered_map<std::string, void*> argv_;  // 参数变量名到变量指针的映射
};
}  // namespace Common
