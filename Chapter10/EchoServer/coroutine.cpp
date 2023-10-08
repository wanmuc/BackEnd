#include "coroutine.h"

#include <assert.h>

#include <iostream>

#include "percentile.hpp"

namespace MyCoroutine {

static bool isBatchDone(Schedule& schedule, int batchId) {
  assert(batchId >= 0 && batchId < MAX_BATCH_RUN_SIZE);
  assert(schedule.batchs[batchId]->state == Run);  // 校验batch的状态，必须是run的状态
  for (const auto& kv : schedule.batchs[batchId]->cid2finish) {
    if (not kv.second) return false;  // 只要有一个关联的协程没执行完，就返回false
  }
  return true;
}

static void CoroutineRun(Schedule* schedule) {
  schedule->isMasterCoroutine = false;
  int id = schedule->runningCoroutineId;
  assert(id >= 0 && id < schedule->coroutineCnt);
  Coroutine* routine = schedule->coroutines[id];
  // 执行entry函数
  routine->entry(routine->arg);
  // entry函数执行完之后，才能把协程状态更新为idle，并标记runningCoroutineId为无效的id
  routine->state = Idle;
  // 如果有关联的batch，则要更新batch的信息，设置batch关联的协程已经执行完
  if (routine->relateBatchId != INVALID_BATCH_ID) {
    Batch* batch = schedule->batchs[routine->relateBatchId];
    batch->cid2finish[id] = true;
    // batch都执行完了，则更新batchFinishList。
    if (isBatchDone(*schedule, routine->relateBatchId)) {
      schedule->batchFinishList.push_back(batch->relateId);
    }
    routine->relateBatchId = INVALID_BATCH_ID;
  }
  schedule->activityCnt--;
  schedule->runningCoroutineId = INVALID_ROUTINE_ID;
  if (schedule->stackCheck) {
    assert(Normal == CoroutineStackCheck(*schedule, id));
  }
  // 这个函数执行完，调用栈会回到主协程中，执行routine->ctx.uc_link指向的上下文的下一条指令
}

static void CoroutineInit(Schedule& schedule, Coroutine* routine, Entry entry, void* arg, uint32_t priority,
                          int relateBatchId) {
  routine->arg = arg;
  routine->entry = entry;
  routine->state = Ready;
  routine->priority = priority;
  routine->relateBatchId = relateBatchId;
  routine->isInsertBatch = false;
  if (nullptr == routine->stack) {
    routine->stack = new uint8_t[schedule.stackSize];
    // 填充栈顶canary内容
    memset(routine->stack, CANARY_PADDING, CANARY_SIZE);
    // 填充栈底canary内容
    memset(routine->stack + schedule.stackSize - CANARY_SIZE, CANARY_PADDING, CANARY_SIZE);
  }
  getcontext(&(routine->ctx));
  routine->ctx.uc_stack.ss_flags = 0;
  routine->ctx.uc_stack.ss_sp = routine->stack + CANARY_SIZE;
  routine->ctx.uc_stack.ss_size = schedule.stackSize - 2 * CANARY_SIZE;
  routine->ctx.uc_link = &(schedule.main);
  // 设置routine->ctx上下文要执行的函数和对应的参数，
  // 这里没有直接使用entry和arg设置，而是多包了一层CoroutineRun函数的调用，
  // 是为了在CoroutineRun中entry函数执行完之后，从协程的状态更新Idle，并更新当前处于运行中的从协程id为无效id，
  // 这样这些逻辑就可以对上层调用透明。
  makecontext(&(routine->ctx), (void (*)(void))(CoroutineRun), 1, &schedule);
}

int CoroutineCreate(Schedule& schedule, Entry entry, void* arg, uint32_t priority, int relateBatchId) {
  int id = 0;
  for (id = 0; id < schedule.coroutineCnt; id++) {
    if (schedule.coroutines[id]->state == Idle) break;
  }
  if (id >= schedule.coroutineCnt) {
    return INVALID_ROUTINE_ID;
  }
  schedule.activityCnt++;
  Coroutine* routine = schedule.coroutines[id];
  CoroutineInit(schedule, routine, entry, arg, priority, relateBatchId);
  return id;
}

bool CoroutineCanCreate(Schedule& schedule) {
  int id = 0;
  for (id = 0; id < schedule.coroutineCnt; id++) {
    if (schedule.coroutines[id]->state == Idle) return true;
  }
  return false;
}

void CoroutineYield(Schedule& schedule) {
  assert(not schedule.isMasterCoroutine);
  int id = schedule.runningCoroutineId;
  assert(id >= 0 && id < schedule.coroutineCnt);
  Coroutine* routine = schedule.coroutines[schedule.runningCoroutineId];
  // 更新当前的从协程状态为挂起
  routine->state = Suspend;
  // 当前的从协程让出执行权，并把当前的从协程的执行上下文保存到routine->ctx中，
  // 执行权回到主协程中，主协程再做调度，当从协程被主协程resume时，swapcontext才会返回。
  swapcontext(&routine->ctx, &(schedule.main));
  schedule.isMasterCoroutine = false;
}

int CoroutineResume(Schedule& schedule) {
  assert(schedule.isMasterCoroutine);
  bool isInsertBatch = true;
  uint32_t priority = UINT32_MAX;
  int coroutineId = INVALID_ROUTINE_ID;
  // 按优先级调度，选择优先级最高的状态为挂起的从协程来运行，并考虑是否插入了batch卡点
  for (int i = 0; i < schedule.coroutineCnt; i++) {
    if (schedule.coroutines[i]->state == Idle || schedule.coroutines[i]->state == Run) {
      continue;
    }
    // 执行到这里，schedule.coroutines[i]->state的值为 Suspend 或者 Ready
    // 没batch卡点的协程优先级更高
    if (not schedule.coroutines[i]->isInsertBatch && isInsertBatch) {
      coroutineId = i;
      priority = schedule.coroutines[i]->priority;
      isInsertBatch = false;
    } else if (schedule.coroutines[i]->isInsertBatch && not isInsertBatch) {
      // 插入batch卡点的协程优先级更低，所以这里不更新isInsertBatch，priority，coroutineId
    } else {  // 都没插入batch卡点 或者 都插入了batch卡点
      if (schedule.coroutines[i]->priority < priority) {
        coroutineId = i;
        priority = schedule.coroutines[i]->priority;
      }
    }
  }

  if (coroutineId == INVALID_ROUTINE_ID) return NotRunnable;
  Coroutine* routine = schedule.coroutines[coroutineId];
  // 如果是被插入batch卡点的协程需要再校验batch是否执行完
  if (isInsertBatch) {
    // batch卡点关联的协程必须全部执行完
    assert(isBatchDone(schedule, routine->relateBatchId));
  }
  routine->state = Run;
  schedule.runningCoroutineId = coroutineId;
  // 从主协程切换到协程编号为id的协程中执行，并把当前执行上下文保存到schedule.main中，
  // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
  swapcontext(&schedule.main, &routine->ctx);
  schedule.isMasterCoroutine = true;
  return Success;
}

int CoroutineResumeById(Schedule& schedule, int id) {
  assert(schedule.isMasterCoroutine);
  assert(id >= 0 && id < schedule.coroutineCnt);
  Coroutine* routine = schedule.coroutines[id];
  // 挂起状态或者就绪状态的的协程才可以唤醒
  if (routine->state != Suspend && routine->state != Ready) return NotRunnable;
  // 有被插入batch卡点的，需要batch执行完才可以唤醒
  if (routine->isInsertBatch && not isBatchDone(schedule, routine->relateBatchId)) return NotRunnable;
  routine->state = Run;
  schedule.runningCoroutineId = id;
  // 从主协程切换到协程编号为id的协程中执行，并把当前执行上下文保存到schedule.main中，
  // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
  swapcontext(&schedule.main, &routine->ctx);
  schedule.isMasterCoroutine = true;
  return Success;
}

int CoroutineResumeInBatch(Schedule& schedule, int id) {
  assert(schedule.isMasterCoroutine);
  assert(id >= 0 && id < schedule.coroutineCnt);
  Coroutine* routine = schedule.coroutines[id];
  // 没有被插入batch卡点，则没有需要唤醒的batch协程
  if (not routine->isInsertBatch) return NotRunnable;
  int batchId = routine->relateBatchId;
  auto iter = schedule.batchs[batchId]->cid2finish.begin();
  // 恢复batch关联的所有从协程
  while (iter != schedule.batchs[batchId]->cid2finish.end()) {
    assert(iter->second == false);
    assert(CoroutineResumeById(schedule, iter->first) == Success);
    iter++;
  }
  return Success;
}

int CoroutineResumeBatchFinish(Schedule& schedule) {
  assert(schedule.isMasterCoroutine);
  if (schedule.batchFinishList.size() <= 0) return NotRunnable;
  while (not schedule.batchFinishList.empty()) {
    int cid = schedule.batchFinishList.front();
    schedule.batchFinishList.pop_front();
    assert(CoroutineResumeById(schedule, cid) == Success);
  }
  return Success;
}

bool CoroutineIsInBatch(Schedule& schedule) {
  assert(not schedule.isMasterCoroutine);
  int cid = schedule.runningCoroutineId;
  return schedule.coroutines[cid]->relateBatchId != INVALID_BATCH_ID;
}

void CoroutineLocalSet(Schedule& schedule, void* key, LocalData localData) {
  assert(not schedule.isMasterCoroutine);  // 从协程中才可以调用
  int cid = schedule.runningCoroutineId;
  auto iter = schedule.coroutines[cid]->local.find(key);
  if (iter != schedule.coroutines[cid]->local.end()) {
    iter->second.freeEntry(iter->second.data);  // 之前有值，则要先释放空间
  }
  schedule.coroutines[cid]->local[key] = localData;
}

bool CoroutineLocalGet(Schedule& schedule, void* key, LocalData& localData) {
  assert(not schedule.isMasterCoroutine);  // 从协程中才可以调用
  int cid = schedule.runningCoroutineId;
  auto iter = schedule.coroutines[cid]->local.find(key);
  if (iter == schedule.coroutines[cid]->local.end()) {
    // 如果不存在，判断是否有关联batch，
    int relateBatchId = schedule.coroutines[cid]->relateBatchId;
    if (relateBatchId == INVALID_BATCH_ID) return false;
    // 从被插入batch卡点的协程中查找，进而实现部分协程间本地变量的共享
    Batch* batch = schedule.batchs[relateBatchId];
    iter = schedule.coroutines[batch->relateId]->local.find(key);
    if (iter == schedule.coroutines[batch->relateId]->local.end()) return false;
    localData = iter->second;
    return true;
  }
  localData = iter->second;
  return true;
}

int CoroutineStackCheck(Schedule& schedule, int id) {
  assert(id >= 0 && id < schedule.coroutineCnt);
  Coroutine* routine = schedule.coroutines[id];
  assert(routine->stack);
  // 栈的"生长"方向，是从高地址到低地址
  for (int i = 0; i < CANARY_SIZE; i++) {
    if (routine->stack[i] != CANARY_PADDING) {
      return OverFlow;
    }
    if (routine->stack[schedule.stackSize - 1 - i] != CANARY_PADDING) {
      return UnderFlow;
    }
  }
  return Normal;
}

int BatchInit(Schedule& schedule) {
  assert(not schedule.isMasterCoroutine);  // 从协程中才可以调用
  for (int i = 0; i < MAX_BATCH_RUN_SIZE; i++) {
    if (schedule.batchs[i]->state == Idle) {
      schedule.batchs[i]->state = Ready;
      schedule.batchs[i]->relateId = schedule.runningCoroutineId;
      schedule.coroutines[schedule.runningCoroutineId]->relateBatchId = i;
      schedule.coroutines[schedule.runningCoroutineId]->isInsertBatch = true;
      return i;
    }
  }
  return INVALID_BATCH_ID;
}

void BatchAdd(Schedule& schedule, int batchId, Entry entry, void* arg, uint32_t priority) {
  assert(not schedule.isMasterCoroutine);  // 从协程中才可以调用
  assert(batchId >= 0 && batchId < MAX_BATCH_RUN_SIZE);  // 校验batchId的合法性
  assert(schedule.batchs[batchId]->state == Ready);  // batch必须是ready的状态
  assert(schedule.batchs[batchId]->relateId == schedule.runningCoroutineId);  // 关联的协程id必须正确
  int id = CoroutineCreate(schedule, entry, arg, priority, batchId);
  assert(id != INVALID_ROUTINE_ID);
  schedule.batchs[batchId]->cid2finish[id] = false;  // 新增要执行的协程还没执行完
}

void BatchRun(Schedule& schedule, int batchId) {
  assert(not schedule.isMasterCoroutine);  // 从协程中才可以调用
  assert(batchId >= 0 && batchId < MAX_BATCH_RUN_SIZE);  // 校验batchId的合法性
  assert(schedule.batchs[batchId]->relateId == schedule.runningCoroutineId);  // 关联的协程id必须正确
  schedule.batchs[batchId]->state = Run;
  CoroutineYield(schedule);  // 这里的BatchRun只是一个卡点，等batch中所有的协程都执行完了，主协程再恢复从协程的执行
  schedule.batchs[batchId]->state = Idle;
  schedule.batchs[batchId]->cid2finish.clear();
  schedule.coroutines[schedule.runningCoroutineId]->relateBatchId = INVALID_BATCH_ID;
  schedule.coroutines[schedule.runningCoroutineId]->isInsertBatch = false;  // 重新设置未被插入batch卡点
}

int ScheduleInit(Schedule& schedule, int coroutineCnt, int stackSize) {
  assert(coroutineCnt > 0 && coroutineCnt <= MAX_COROUTINE_SIZE);  // 最多创建MAX_COROUTINE_SIZE个协程
  stackSize += (CANARY_SIZE * 2);  // 添加canary需要的额外内存
  schedule.activityCnt = 0;
  schedule.stackCheck = true;
  schedule.stackSize = stackSize;
  schedule.isMasterCoroutine = true;
  schedule.coroutineCnt = coroutineCnt;
  schedule.runningCoroutineId = INVALID_ROUTINE_ID;
  for (int i = 0; i < coroutineCnt; i++) {
    schedule.coroutines[i] = new Coroutine;
    schedule.coroutines[i]->state = Idle;
    schedule.coroutines[i]->stack = nullptr;
  }
  for (int i = 0; i < MAX_BATCH_RUN_SIZE; i++) {
    schedule.batchs[i] = new Batch;
    schedule.batchs[i]->state = Idle;
  }
  return 0;
}

bool ScheduleRunning(Schedule& schedule) {
  assert(schedule.isMasterCoroutine);
  if (schedule.runningCoroutineId != INVALID_ROUTINE_ID) return true;
  for (int i = 0; i < schedule.coroutineCnt; i++) {
    if (schedule.coroutines[i]->state != Idle) return true;
  }
  return false;
}

void ScheduleClean(Schedule& schedule) {
  assert(schedule.isMasterCoroutine);
  for (int i = 0; i < schedule.coroutineCnt; i++) {
    delete[] schedule.coroutines[i]->stack;
    for (auto& item : schedule.coroutines[i]->local) {
      item.second.freeEntry(item.second.data);  // 释放协程本地变量的内存空间
    }
    delete schedule.coroutines[i];
  }
  for (int i = 0; i < MAX_BATCH_RUN_SIZE; i++) {
    delete schedule.batchs[i];
  }
}

bool ScheduleTryReleaseMemory(Schedule& schedule) {
  static Percentile pct;
  pct.Stat("activityCnt", schedule.activityCnt);
  double pctValue;
  // 保持pct99的水位即可
  if (not pct.GetPercentile("activityCnt", 0.99, pctValue)) return false;
  int32_t releaseCnt = 0;
  // 扣除活动的协程，计算剩余需要保留的栈空间内存的协程数
  int32_t remainStackCnt = (int32_t)pctValue - schedule.activityCnt;
  for (int i = 0; i < schedule.coroutineCnt; i++) {
    if (schedule.coroutines[i]->state != Idle) continue;
    if (nullptr == schedule.coroutines[i]->stack) continue;
    if (remainStackCnt <= 0) {  // 没有保留名额了
      delete[] schedule.coroutines[i]->stack;  // 释放状态为idle的协程的栈内存
      schedule.coroutines[i]->stack = nullptr;
      for (auto& item : schedule.coroutines[i]->local) {
        item.second.freeEntry(item.second.data);  // 释放协程本地变量的内存空间
      }
      schedule.coroutines[i]->local.clear();
      releaseCnt++;
      if (releaseCnt >= 25) break;  // 每次最多释放25个协程栈的空间，避免释放内存占用过多时间
    } else {
      remainStackCnt--;  // 保留名额减1
    }
  }
  return true;
}

int ScheduleGetRunCid(Schedule& schedule) { return schedule.runningCoroutineId; }
void ScheduleDisableStackCheck(Schedule& schedule) { schedule.stackCheck = false; }

}  // namespace MyCoroutine