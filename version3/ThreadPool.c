#include <stdlib.h>
#include "ThreadPool.h"
#include <assert.h>

struct ThreadPool* threadPollInit(struct EventLoop* mainLoop, int count)
{
  struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
  pool->index = 0;
  pool->isStart = false;
  pool->mainLopp = mainLoop;
  pool->threadNum = count;
  pool->workerThreads = (struct WorkThread*)malloc(sizeof(struct WorkThread) * count);

  return pool;
}

void threadPoolRun(struct ThreadPool* pool)
{
  assert(pool && !pool->isStart);
  if(pool->mainLopp->threadID == pthread_self())
  {

  }

}

