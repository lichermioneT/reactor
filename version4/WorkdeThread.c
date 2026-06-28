#include "WorkdeThread.h"

#include <stdio.h>

int workerThreadInit(struct WorkThread* thread, int index)
{
  if(thread == NULL)
  {
    return -1;
  }

  thread->evLoop = NULL;
  thread->threadID = 0;
  snprintf(thread->name, sizeof(thread->name), "SubThread-%d", index);

  pthread_mutex_init(&thread->mutex, NULL);
  pthread_cond_init(&thread->cond, NULL);

  return 0;
}

static void* subThreadRunning(void* arg)
{
  struct WorkThread* thread = (struct WorkThread*)arg;

  pthread_mutex_lock(&thread->mutex);
  thread->evLoop = EventLoopInitEx(thread->name);
  pthread_cond_signal(&thread->cond);
  pthread_mutex_unlock(&thread->mutex);

  eventLoopRun(thread->evLoop);
  return thread;
}

void workerThreadRun(struct WorkThread* thread)
{
  if(thread == NULL)
  {
    return;
  }

  int ret = pthread_create(&thread->threadID, NULL, subThreadRunning, thread);
  if(ret != 0)
  {
    return;
  }

  pthread_mutex_lock(&thread->mutex);
  while(thread->evLoop == NULL)
  {
    pthread_cond_wait(&thread->cond, &thread->mutex);
  }
  pthread_mutex_unlock(&thread->mutex);
}
