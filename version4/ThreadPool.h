#pragma once

#include "EventLoop.h"
#include "WorkdeThread.h"

#include <stdbool.h>

struct ThreadPool
{
  struct EventLoop* mainLopp;
  bool isStart;
  int threadNum;
  struct WorkThread* workerThreads;
  int index;
};

// MODIFIED: restored ThreadPool struct and API declarations.
struct ThreadPool* threadPollInit(struct EventLoop* mainLoop, int count);
void threadPoolRun(struct ThreadPool* pool);
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool);
