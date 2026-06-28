#pragma once

#include "ChannelMap.h"
#include "Dispatcher.h"

#include <pthread.h>
#include <stdbool.h>

extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

enum ElemType
{
  ADD,
  DELETE,
  MODIFY
};

struct ChannelElement
{
  int type;
  struct Channel* channel;
  struct ChannelElement* next;
};

struct EventLoop
{
  bool isQuit;
  struct Dispatcher* dispatcher;
  void* dispatcherdata;

  struct ChannelElement* head;
  struct ChannelElement* tail;
  struct ChannelMap* channelMap;

  pthread_t threadID;
  char threadName[32];
  pthread_mutex_t mutex;
  int socketPair[2];
};

// MODIFIED: restored EventLoop declarations and struct fields.
struct EventLoop* EventLoopInit();
struct EventLoop* EventLoopInitEx(const char* threadName);
int eventLoopRun(struct EventLoop* evLoop);
int eventActivate(struct EventLoop* evLoop, int fd, int event);
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);
int eventLoopProcessTask(struct EventLoop* evLoop);
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);
