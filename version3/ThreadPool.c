#include <stdlib.h>
#include "ThreadPool.h"
#include <assert.h>

// 1.初始化线程池所需要的变量
struct ThreadPool* threadPollInit(struct EventLoop* mainLoop, int count)
{
  struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
  pool->index = 0;
  pool->isStart = false;
  pool->mainLopp = mainLoop;
  pool->threadNum = count;

  // 为线程池需要的变量开辟堆内存的。
  pool->workerThreads = (struct WorkThread*)malloc(sizeof(struct WorkThread) * count);

  return pool;
}

// 2.主线程和子进程启动
void threadPoolRun(struct ThreadPool* pool)
{
  assert(pool && !pool->isStart);
  if(pool->mainLopp->threadID != pthread_self())
  {
    exit(0);
  }

  pool->isStart = true;
  if(pool->threadNum)
  {
    for(int i = 0; i < pool->threadNum; ++i)
    {
      workerThreadInit(&pool->workerThreads[i], i);
      workerThreadRun(&pool->workerThreads[i]);
    }
  }
}

// 3.线程池里面哪一个线程的反应堆模型
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
{
  // 主线程拥有线程池
  // 判断是否运行
  assert(pool->isStart);
  
  if(pool->mainLopp->threadID != pthread_self())
  {
    exit(0);
  }

  // 从线程池里面找一个子进程，然后取出反应堆的实例
  struct EventLoop* evLoop = pool->mainLopp;
  if(pool->threadNum > 0)
  {
    evLoop = pool->workerThreads[pool->index].evLoop;
    // pool->index = ++pool->index % pool->threadNum;
    pool->index = (pool->index + 1) % pool->threadNum;
  }
    
  // 返回
  return evLoop;
}
