#pragma once

#include "coroutine.h"

namespace Core {
// 协程本地变量模版类
template <class Type>
class CoroutineLocal {
 public:
  static void FreeLocal(void* data) {
    if (data) delete (Type*)data;
  }
  void Set(Type value) {
    Type* temp = new Type;
    *temp = value;
    MyCoroutine::LocalData localData{
        .data = temp,
        .freeEntry = FreeLocal,
    };
    MyCoroutine::CoroutineLocalSet(SCHEDULE, this, localData);
  }
  Type& Get() {
    MyCoroutine::LocalData localData;
    bool result = MyCoroutine::CoroutineLocalGet(SCHEDULE, this, localData);
    assert(result == true);
    return *(Type*)localData.data;
  }
};
}  // namespace Core