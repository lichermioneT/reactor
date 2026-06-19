#include "WorkdeThread.h"
#include <stdio.h>

int workerThreadInit(struct WorkThread* thread, int index)
{
  thread->evLoop = NULL;
  thread->threadID = 0;
  sprintf(thread->name, "SubThread-%d", index);

  pthread_mutex_init(&thread->mutex, NULL);
  pthread_cond_init(&thread->cond, NULL);

  return 0;
}

// 子线程的回调函数
void* subThreadRunning(void* arg)
{
  struct WorkThread* thread = (struct WorkThread*)arg;

  pthread_mutex_lock(&thread->mutex);
  thread->evLoop = EventLoopInitEx(thread->name); // 子线程实例化一个反应堆模型
  pthread_mutex_unlock(&thread->mutex);

  pthread_cond_signal(&thread->cond);

  eventLoopRun(thread->evLoop);

  return NULL;
}

void  workerThreadRun(struct WorkThread* thread)
{
  // 1.创建子线程
  pthread_create(&thread->threadID, NULL, subThreadRunning, thread);

  // 2.主线程阻塞一会,
  pthread_mutex_lock(&thread->mutex);
  while(thread->evLoop == NULL)
  {
    pthread_cond_wait(&thread->cond, &thread->mutex);
  }
  pthread_mutex_unlock(&thread->mutex);
}
