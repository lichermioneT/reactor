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
  bool isQuit;                    // 判断是否退出
  struct Dispatcher* dispatcher;  // IO复用的模型
  void* dispatcherdata;           // IO复用的模型，需要的辅助空间

  struct ChannelElement* head;    // 任务队列的头
  struct ChannelElement* tail;    // 任务队列的尾巴
  struct ChannelMap* channelMap;  // map指针数组

  pthread_t threadID;             // 线程ID
  char threadName[32];            // 线程name
  pthread_mutex_t mutex;          // 互斥锁
  int socketPair[2];              // 唤醒的文件描述符
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
