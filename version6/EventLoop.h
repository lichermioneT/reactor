#pragma once
#include <pthread.h>
#include "ChannelMap.h"
#include "Dispatcher.h"
#include "Channel.h"
#include <stdbool.h>

extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher SelectDispatcher;
extern struct Dispatcher PollDispatcher;

enum ElemType{ADD, DELETE, MOD};

// 定义任务队列节点
struct ChannelElement
{
  int type; // add,mod,del
  struct Channel* channel;
  struct ChannelElement* next;
};

struct EventLoop
{
// 0.模型开关
  bool isQuit;
// 1.分发模型
  struct Dispatcher* dispatcher; // 事件分发模型
  void* dispatcherData;   // 分发模型需要的空间
// 2.任务队列(增，删，修)
  struct ChannelElement* head; 
  struct ChannelElement* tail;
// 3.ChannelMap
  struct ChannelMap* map;
// 4.thread info
  pthread_t threadID;
  char threadName[32];
  pthread_mutex_t mutex;
// 5.线程间通信
  int socketPair[2]; // 
};

// 初始化
struct EventLoop* eventLoopInit();
struct EventLoop* eventLoopInitEx(const char* threadName);
// 启动反应堆模型
int eventLoopRun(struct EventLoop* evLoop);
// 处理激活文件描述符的函数
int eventActivate(struct EventLoop* evLoop, int fd, int event);
// 添加任务到任务队列
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);
// 处理任务中的任务
int eventLoopProcessTask(struct EventLoop* evLoop);
// 处理dispatcher中的节点
int eventLoopADD(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);
// 释放channel
int destoryChannel(struct EventLoop* evLoop, struct Channel* channel);
