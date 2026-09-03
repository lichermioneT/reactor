#include "ThreadPool.h"
#include <assert.h>
#include <stdlib.h>

struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count)
{
  struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
  pool->index = 0;
  pool->isStart = false;
  pool->mainLoop = mainLoop;
  pool->trheadNum = count;
  pool->workerThreads = (struct WorkerThread*)malloc(sizeof(struct WorkerThread)*count);
  return pool;
}

void threadPoolRun(struct ThreadPool* pool)
{
  assert(pool && !pool->isStart);
  if(pool->mainLoop->threadID != pthread_self())
  {
    exit(0);
  }

  pool->isStart = true;

  if(pool->trheadNum)
  {
    for(int i = 0; i < pool->trheadNum; ++i)
    {
      workerThreadInit(&pool->workerThreads[i], i);
      workerThreadRun(&pool->workerThreads[i]);
    }
  }
}

// 子线程的反应堆模型(其实和主线程的一模一样的)
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
{
  assert(pool->isStart); 
  if(pool->mainLoop->threadID != pthread_self())
  {
    exit(0);
  }

  // 找一个子线程
  struct EventLoop* evLoop = pool->mainLoop;
  if(pool->trheadNum > 0)
  {
      evLoop = pool->workerThreads[pool->index].evLoop;
      pool->index = (++pool->index) % pool->trheadNum;
  }

  return  evLoop;
}
