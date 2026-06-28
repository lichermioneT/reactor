#pragma once

#include "EventLoop.h"

#include <pthread.h>

struct WorkThread
{
  pthread_t threadID;
  char name[24];
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  struct EventLoop* evLoop;
};

// MODIFIED: restored WorkThread fields and declarations.
int workerThreadInit(struct WorkThread* thread, int index);
void workerThreadRun(struct WorkThread* thread);
