#pragma once

#include "EventLoop.h"
#include "WorkdeThread.h"

#include <stdbool.h>

/*
 * Reactor 工作线程池。
 *
 * 负责管理多个 WorkThread，并将主 Reactor 接收到的新连接
 * 按照一定策略分配给不同的子 Reactor。
 */

struct ThreadPool
{
  struct EventLoop* mainLoop; // 主线程所属的事件循环，即主 Reactor
  bool isStart;                // 线程池是否已经启动
  int threadNum;               // 工作线程数量
  struct WorkThread* workerThreads;    // 工作线程数组，每个线程拥有一个子 EventLoop
  int index;                           // 下一次分配连接时选用的工作线程下标
};

// MODIFIED: restored ThreadPool struct and API declarations.
struct ThreadPool* threadPollInit(struct EventLoop* mainLoop, int count);
void threadPoolRun(struct ThreadPool* pool);
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool);
