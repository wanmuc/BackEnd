#include <assert.h>

#include <iostream>

#include "../coroutine.h"
using namespace std;

void freeLocal(void *data) {
  if (data) delete (int *)data;
}

void LocalSet(MyCoroutine::Schedule &schedule, int *key, int value) {
  int *temp = new int;
  *temp = value;
  MyCoroutine::LocalData localData{
      .data = temp,
      .freeEntry = freeLocal,
  };
  MyCoroutine::CoroutineLocalSet(schedule, key, localData);
}

int &LocalGet(MyCoroutine::Schedule &schedule, int *key) {
  MyCoroutine::LocalData localData;
  bool result = MyCoroutine::CoroutineLocalGet(schedule, key, localData);
  assert(result == true);
  return *(int *)localData.data;
}

int localDataKey;

void routine1(void *arg) {
  cout << "routine1 run begin" << endl;
  MyCoroutine::Schedule *schedule = (MyCoroutine::Schedule *)arg;
  LocalSet(*schedule, &localDataKey, 666);
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine1 running" << endl;
  MyCoroutine::CoroutineYield(*schedule);

  int &value = LocalGet(*schedule, &localDataKey);
  assert(value == 666);
  cout << "routine1 CoroutineLocalGet value is valid" << endl;
  cout << "routine1 run end" << endl;
}

void batchRunTask1(void *arg) { cout << "batchRunTask1 deal" << endl; }

void batchRunTask2(void *arg) { cout << "batchRunTask2 deal" << endl; }

void routine2(void *arg) {
  cout << "routine2 run begin" << endl;
  MyCoroutine::Schedule *schedule = (MyCoroutine::Schedule *)arg;
  LocalSet(*schedule, &localDataKey, 888);
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine2 running" << endl;
  MyCoroutine::CoroutineYield(*schedule);

  int batchId = MyCoroutine::BatchInit(*schedule);
  MyCoroutine::BatchAdd(*schedule, batchId, batchRunTask1, schedule);
  MyCoroutine::BatchAdd(*schedule, batchId, batchRunTask2, schedule);
  MyCoroutine::BatchRun(*schedule, batchId);

  batchId = MyCoroutine::BatchInit(*schedule);
  MyCoroutine::BatchAdd(*schedule, batchId, batchRunTask1, schedule);
  MyCoroutine::BatchAdd(*schedule, batchId, batchRunTask2, schedule);
  MyCoroutine::BatchRun(*schedule, batchId);

  int &value = LocalGet(*schedule, &localDataKey);
  assert(value == 888);
  cout << "routine2 CoroutineLocalGet value is valid" << endl;
  cout << "routine2 run end" << endl;
}

int main() {
  MyCoroutine::Schedule schedule;
  MyCoroutine::ScheduleInit(schedule, 100);
  MyCoroutine::CoroutineCreate(schedule, routine1, &schedule, 1);
  MyCoroutine::CoroutineCreate(schedule, routine2, &schedule, 2);
  while (MyCoroutine::ScheduleRunning(schedule)) {
    MyCoroutine::CoroutineResume(schedule);
  }
  MyCoroutine::ScheduleClean(schedule);
  return 0;
}
