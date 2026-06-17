#include "EventLoop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct EventLoop* EventLoopInit()
{
  return EventLoopInitEx(NULL);
}

struct EventLoop* EventLoopInitEx(const char* threadName)
{
  struct EventLoop*  evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
  if(evLoop == NULL)
  {
    perror("malloc");
    return NULL;
  }

  evLoop->isQuit = false;
  evLoop->threadID = pthread_self();
  pthread_mutex_init(&evLoop->mutex, NULL);
  strcpy(evLoop->threadName, threadName == NULL ? "main thread" : threadName);
  evLoop->dispatcher = &EpollDispatcher;
  evLoop->dispatcherdata = evLoop->dispatcher->init();
  
  // 链表
  evLoop->head = evLoop->tail = NULL;
  // map
  evLoop->channelMap = channelMapInit(128);
  
  return evLoop;
}
