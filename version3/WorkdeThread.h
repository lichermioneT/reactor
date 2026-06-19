#pragma once
#include "EventLoop.h"
#include <pthread.h>

// 定义子线程对应的结构体
struct WorkThread
{
  // 1.线程属性
  pthread_t threadID; // 线程的ID
  char name[24];      // 线程的名字
  // 2.线程同步和互斥
  pthread_mutex_t mutex; // 互斥锁
  pthread_cond_t cond;   // 条件变量(计数器)
  // 3.反应堆模型
  struct EventLoop* evLoop; // 反应堆模型
};

// 1.创建线程的示例
int workerThreadInit(struct WorkThread* thread, int index);

// 2.启动线程
void  workerThreadRun(struct WorkThread* thread);
