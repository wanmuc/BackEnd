#pragma once

#include <functional>

namespace Common {
class Defer {
 public:
  Defer(std::function<void(void)> func) : func_(func) {}
  ~Defer() { func_(); }

 private:
  std::function<void(void)> func_;
};
}  // namespace Common