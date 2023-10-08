#pragma once

#include "coroutine.h"

namespace Core {
class WaitGroup {
 public:
  WaitGroup() { batch_id_ = MyCoroutine::BatchInit(SCHEDULE); }
  void Add(MyCoroutine::Entry entry, void* arg) { MyCoroutine::BatchAdd(SCHEDULE, batch_id_, entry, arg); }
  void Wait() { MyCoroutine::BatchRun(SCHEDULE, batch_id_); }

 private:
  int batch_id_;  // 批量运行的id
};
}  // namespace Core