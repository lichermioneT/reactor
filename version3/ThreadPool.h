#pragma once
#include <stdbool.h>
#include "EventLoop.h"
#include "WorkdeThread.h"

// 定义线程池
struct ThreadPool 
{
  //主线程的反应堆模型
  struct EventLoop* mainLopp;
  bool isStart;
  int threadNum;
  struct WorkThread* workerThreads;
  int index;
};

// 初始化线程池
struct ThreadPool* threadPollInit(struct EventLoop* mainLoop, int count);
// 启动线程池
void threadPoolRun(struct ThreadPool* pool);
// 取出线程池中的某个子线程的反应堆示例
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool);
