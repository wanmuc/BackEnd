#pragma once

#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include <cstdint>
#include <list>
#include <unordered_map>

namespace MyCoroutine {

constexpr int INVALID_BATCH_ID = -1;        // 无效的batchId
constexpr int INVALID_ROUTINE_ID = -1;      // 无效的协程id
constexpr int MAX_COROUTINE_SIZE = 102400;  // 最多创建102400个协程
constexpr int MAX_BATCH_RUN_SIZE = 51200;   // 最多创建51200个批量执行
constexpr int CANARY_SIZE = 512;            // canary内存的大小，单位字节
constexpr uint8_t CANARY_PADDING = 0x88;    // canary填充的内容

/* 1.协程的状态，协程的状态转移如下：
 *  idle->ready
 *  ready->run
 *  run->suspend
 *  suspend->run
 *  run->idle
 * 2.批量执行的状态，批量执行的状态转移如下：
 *  idle->ready
 *  ready->run
 *  run->idle
 */
enum State {
  Idle = 1,     // 空闲
  Ready = 2,    // 就绪
  Run = 3,      // 运行
  Suspend = 4,  // 挂起
};

enum ResumeResult {
  NotRunnable = 1,  // 无可运行的协程
  Success = 2,      // 成功唤醒一个挂起状态的协程
};

enum StackCheckResult {
  Normal = 0,     // 正常
  OverFlow = 1,   // 栈顶溢出
  UnderFlow = 2,  // 栈底溢出
};

// 入口函数
typedef void (*Entry)(void* arg);

// 协程本地变量数据
typedef struct LocalData {
  void* data;
  Entry freeEntry;  // 用于释放本地协程变量的内存
} LocalData;

// 协程结构体
typedef struct Coroutine {
  State state;                                 // 协程当前的状态
  uint32_t priority;                           // 协程优先级，值越小，优先级越高
  void* arg;                                   // 协程入口函数的参数
  Entry entry;                                 // 协程入口函数
  ucontext_t ctx;                              // 协程执行上下文
  uint8_t* stack;                              // 每个协程独占的协程栈，动态分配
  std::unordered_map<void*, LocalData> local;  // 协程本地变量，key是协程变量的内存地址
  int relateBatchId;                           // 关联的batchId，INVALID_BATCH_ID表示无关联的batch
  bool isInsertBatch;                          // 当前在协程中是否被插入了batchRun的卡点
} Coroutine;

// 批量执行结构体
typedef struct Batch {
  State state;                               // 批量执行的状态
  int relateId;                              // 关联的协程id
  std::unordered_map<int, bool> cid2finish;  // 每个关联协程的运行状态（是否执行完）
} Batch;

// 协程调度器
typedef struct Schedule {
  ucontext_t main;                            // 用于保存主协程的上下文
  int32_t runningCoroutineId;                 // 运行中（Run + Suspend）的从协程的id
  int32_t coroutineCnt;                       // 协程个数
  int32_t activityCnt;                        // 非idle状态的协程数
  bool isMasterCoroutine;                     // 当前协程是否为主协程
  Coroutine* coroutines[MAX_COROUTINE_SIZE];  // 从协程数组池
  Batch* batchs[MAX_BATCH_RUN_SIZE];          // 批量执行数组池
  int stackSize;                              // 协程栈的大小，单位字节
  std::list<int> batchFinishList;             // 完成了批量执行的关联的协程的id
  bool stackCheck;                            // 是否检测协程栈空间是否溢出
} Schedule;

// 创建协程
int CoroutineCreate(Schedule& schedule, Entry entry, void* arg, uint32_t priority = 0,
                    int relateBatchId = INVALID_BATCH_ID);
// 判断是否可以创建协程
bool CoroutineCanCreate(Schedule& schedule);
// 让出执行权，只能在从协程中调用
void CoroutineYield(Schedule& schedule);
// 恢复从协程的调用，只能在主协程中调用
int CoroutineResume(Schedule& schedule);
// 恢复指定从协程的调用，只能在主协程中调用
int CoroutineResumeById(Schedule& schedule, int id);
// 恢复从协程batch中协程的调用，只能在主协程中调用
int CoroutineResumeInBatch(Schedule& schedule, int id);
// 恢复被插入batch卡点的从协程的调用，只能在主协程中调用
int CoroutineResumeBatchFinish(Schedule& schedule);
// 判断当前从协程是否在batch中
bool CoroutineIsInBatch(Schedule& schedule);
// 设置协程本地变量
void CoroutineLocalSet(Schedule& schedule, void* key, LocalData localData);
// 获取协程本地变量
bool CoroutineLocalGet(Schedule& schedule, void* key, LocalData& localData);
// 协程栈使用检测
int CoroutineStackCheck(Schedule& schedule, int id);

// 初始化一个批量执行的上下文
int BatchInit(Schedule& schedule);
// 在批量执行上下文中添加要执行的任务
void BatchAdd(Schedule& schedule, int batchId, Entry entry, void* arg, uint32_t priority = 0);
// 执行批量操作
void BatchRun(Schedule& schedule, int batchId);

// 协程调度结构体初始化
int ScheduleInit(Schedule& schedule, int coroutineCnt, int stackSize = 8 * 1024);
// 判断是否还有协程在运行
bool ScheduleRunning(Schedule& schedule);
// 释放调度器
void ScheduleClean(Schedule& schedule);
// 调度器尝试释放内存
bool ScheduleTryReleaseMemory(Schedule& schedule);
// 获取当前运行中的从协程id
int ScheduleGetRunCid(Schedule& schedule);
// 关闭协程栈检查
void ScheduleDisableStackCheck(Schedule& schedule);

}  // namespace MyCoroutine
