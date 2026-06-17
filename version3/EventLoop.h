#pragma once
#include <pthread.h>
#include <stdbool.h>
#include "Dispatcher.h"
#include "ChannelMap.h"

// epoll的分发
extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

enum ElemTyoe{ADD, DELECT, MODIFY};

// 定义任务队列的节点
struct ChannelElement 
{
  int type;               // 如何处理节点中的channel
  struct Channel* channel; // channel是fd的封装
  struct ChannelElement* next; // 下一个节点信息
};

struct EventLoop
{
  bool isQuit; // 是否工作
  struct Dispatcher* dispatcher; // 增删查改的方法epoll,poll,select.
  void* dispatcherdata;          // 增删查改的方法的数据

  // 任务队列
  struct ChannelElement* head;
  struct ChannelElement* tail;
  // map 
  struct ChannelMap* channelMap; 
  // 线程id,name, 互斥锁
  pthread_t threadID;
  char threadName[32];
  pthread_mutex_t mutex;
};

// Dispatcher 是底层事件检测机制的抽象接口；
// EventLoop 是上层事件循环对象，它通过 Dispatcher 去管理和分发 fd 事件

struct EventLoop* EventLoopInit();
struct EventLoop* EventLoopInitEx(const char* threadName);
