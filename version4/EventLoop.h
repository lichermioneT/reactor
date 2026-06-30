#pragma once

#include "ChannelMap.h"
#include "Dispatcher.h"

#include <pthread.h>
#include <stdbool.h>

extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

// 操作类型
enum ElemType
{
  ADD,            // 添加
  DELETE,         // 删除
  MODIFY          // 修改
};

// 任务队列的节点
struct ChannelElement
{
  int type;                     // 操作的类型
  struct Channel* channel;      // 操作的channel
  struct ChannelElement* next;  // 下一个节点
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
// 1.主线程和子线程的反应堆模型的初始化，只有名字不一样的
struct EventLoop* EventLoopInit();
struct EventLoop* EventLoopInitEx(const char* threadName);

// 2.启动evLoop里面的事件分发器，然后变量任务队列
int eventLoopRun(struct EventLoop* evLoop);

// 3.根据event，激活evLoop的fd的读写事件。
int eventActivate(struct EventLoop* evLoop, int fd, int event);

// 4.添加到任务队列里面
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);
// 5.变量任务队列---对map进行add,remove,modify.
int eventLoopProcessTask(struct EventLoop* evLoop);

// 6.对间接添加，移除，修改读或者写.
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);

// 7.释放map对应的channel
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);
