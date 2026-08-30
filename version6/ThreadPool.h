#pragma once 
#include "EventLoop.h"
#include "WorkerThread.h"
#include <stdbool.h>

struct ThreadPool
{
  struct EventLoop* mainLoop; // 建立链接
  bool isStart;
  int trheadNum;
  struct WorkerThread* workerThreads;
  int index;
};

// 初始化线程池
struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count);
// 启动线程池
void threadPoolRun(struct ThreadPool* pool);
// 取一个子线程
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool);
