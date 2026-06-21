#pragma once
#include <pthread.h>
#include <stdbool.h>
#include "Dispatcher.h"
#include "ChannelMap.h"

// epoll, poll, select的分发
extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

// 如何操作事件的操作
enum ElemType{ADD, DELETE, MODIFY};

// 定义任务队列的节点
struct ChannelElement 
{
// 1.ADD,DELETE,MODIFY。从dispatcherdata里面进行执行三种操作。
  int type;                    // 如何处理节点中的channel的文件描述符。
  struct Channel* channel;     // channel是fd的封装
  struct ChannelElement* next; // 下一个节点信息
};

struct EventLoop
{
// 是否工作
  bool isQuit; // 是否工作

// 事件的分发器
  struct Dispatcher* dispatcher; // 增删查改的方法epoll,poll,select.数据分发模型
  void* dispatcherdata;          // 增删查改的方法的数据,epoll,poll,select需要的额外的辅助数据结构

// 如何处理任务队列
// taskQueue任务队列的头结点和尾结点,add, delete, modify的任务。(文件描述符在dispatcherdata里面的。)
  struct ChannelElement* head;
  struct ChannelElement* tail;

// map,事件触发的时候，执行对应的回调函数。
  struct ChannelMap* channelMap; 

// 线程id, name, 互斥锁
  pthread_t threadID;  // 线程id
  char threadName[32]; // 线程名称
  pthread_mutex_t mutex; // 保护共享数据的，任务队列。
  
//存储本地通信的fd，通过socketpiar;
// 唤醒对应的线程执行
  int socketPair[2];
};

// Dispatcher 是底层事件检测机制的抽象接口；
// EventLoop 是上层事件循环对象，它通过 Dispatcher 去管理和分发 fd 事件
// 数据的初始化和操作方法
struct EventLoop* EventLoopInit();
struct EventLoop* EventLoopInitEx(const char* threadName);

// 启动eventloop:反应堆模型，需要告诉我实例是哪个对象
int eventLoopRun(struct EventLoop* evLoop);
// 处理激活的文件fd
int eventActivate(struct EventLoop* evLoop, int fd, int event);

// 添加任务到任务队列里面去
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);

// 处理任务队列中的任务
int eventLoopProcessTask(struct EventLoop* evLoop);

// 处理dispatcher中的节点
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);
