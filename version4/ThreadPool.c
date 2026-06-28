#include "ThreadPool.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct ThreadPool* threadPollInit(struct EventLoop* mainLoop, int count)
{
  if(mainLoop == NULL || count < 0)
  {
    return NULL;
  }

  struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
  if(pool == NULL)
  {
    perror("malloc");
    return NULL;
  }

  pool->index = 0;
  pool->isStart = false;
  pool->mainLopp = mainLoop;
  pool->threadNum = count;
  pool->workerThreads = NULL;

  if(count > 0)
  {
    pool->workerThreads =
        (struct WorkThread*)malloc(sizeof(struct WorkThread) * (size_t)count);
    if(pool->workerThreads == NULL)
    {
      perror("malloc");
      free(pool);
      return NULL;
    }
  }

  return pool;
}

void threadPoolRun(struct ThreadPool* pool)
{
  assert(pool && !pool->isStart);
  if(pool->mainLopp->threadID != pthread_self())
  {
    exit(0);
  }

  pool->isStart = true;
  for(int i = 0; i < pool->threadNum; ++i)
  {
    workerThreadInit(&pool->workerThreads[i], i);
    workerThreadRun(&pool->workerThreads[i]);
  }
}

struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
{
  assert(pool && pool->isStart);

  if(pool->mainLopp->threadID != pthread_self())
  {
    exit(0);
  }

  struct EventLoop* evLoop = pool->mainLopp;
  if(pool->threadNum > 0)
  {
    evLoop = pool->workerThreads[pool->index].evLoop;
    pool->index = (pool->index + 1) % pool->threadNum;
  }

  return evLoop;
}
