#pragma once

namespace Common {
//单例模版类
template <class Type>
class Singleton {
 public:
  static Type& Instance() {
    static Type object;
    return object;
  }
};
}  // namespace Common
