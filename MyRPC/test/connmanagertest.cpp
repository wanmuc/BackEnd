#include "../core/connmanager.hpp"
#include "../core/epollctl.hpp"
#include "../core/timer.hpp"
#include "unittestcore.h"

static int epollFd = 0;

static void ConnTest(void* data) {
  EpollFd.Set(epollFd);
  Core::Conn* conn = CONN_MANAGER.Get("echo");
  assert(conn != nullptr);

  CONN_MANAGER.Put(conn);

  Core::Conn* conn1 = CONN_MANAGER.Get("echo");  // 返回连接池中旧的连接
  assert(conn == conn1);
  CONN_MANAGER.Put(conn1);
  int64_t last_used_time_ = conn1->last_used_time_;

  CONN_MANAGER.SetMaxIdleTime(1);
  sleep(2);
  conn1 = CONN_MANAGER.Get("echo");  // 返回新的连接
  assert(conn1 != nullptr);
  assert(last_used_time_ != conn1->last_used_time_);
  CONN_MANAGER.Release(conn1);

  conn1 = CONN_MANAGER.Get("echo2");
  assert(nullptr == conn1);

  Core::Conn* conns[200];
  for (int i = 0; i < 200; i++) {
    conns[i] = CONN_MANAGER.Get("echo");
    assert(conns[i] != nullptr);
  }
  for (int i = 0; i < 200; i++) {
    if (i < 100) {
      CONN_MANAGER.Put(conns[i]);
    } else {
      CONN_MANAGER.Release(conns[i]);
    }
  }
  *(bool*)data = false;  // 设置成退出循环
}

static void eventHandler(Core::EventData* eventData) {
  int cid = eventData->cid_;
  assert(cid != MyCoroutine::INVALID_ROUTINE_ID);
  MyCoroutine::CoroutineResumeById(SCHEDULE, cid);  // 唤醒之前主动让出cpu的协程
}

TEST_CASE(Connmanager_All) {
  Core::TimerData timerData;
  // echo服务需要先启动
  bool run = true;
  epollFd = epoll_create(1);
  SYSTEM.SetIoMock(nullptr);
  MyCoroutine::ScheduleInit(SCHEDULE, 1024, 64 * 1024);
  MyCoroutine::CoroutineCreate(SCHEDULE, ConnTest, &run);
  MyCoroutine::CoroutineResume(SCHEDULE);
  epoll_event events[2048];
  int msec = -1;
  while (run) {
    bool oneTimer = TIMER.GetLastTimer(timerData);
    if (oneTimer) {
      msec = TIMER.TimeOutMs(timerData);
    }
    int num = epoll_wait(epollFd, events, 2048, msec);
    if (num < 0) {
      continue;
    }
    for (int i = 0; i < num; i++) {
      eventHandler((Core::EventData*)events[i].data.ptr);
    }
    if (oneTimer) {
      TIMER.Run(timerData);
    }
  }
  ASSERT_FALSE(run);
  MyCoroutine::ScheduleClean(SCHEDULE);
}