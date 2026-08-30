#pragma once

#include "EventLoop.h"

#include <pthread.h>

/*
 * 工作线程对象
 *
 * 每个 WorkThread 对应线程池中的一个子线程。
 * 子线程启动后会创建并运行自己的 EventLoop，
 * 负责监听和处理分配给该线程的客户端连接事件。
 */

struct WorkThread
{
  pthread_t threadID;       // 工作线程标识，用于创建、判断和回收线程
  char name[24];            // 工作线程名称，便于日志输出和调试
  pthread_mutex_t mutex;    // 保护工作线程启动状态和 evLoop 指针
  pthread_cond_t cond;      // 用于等待子线程完成 EventLoop 的初始化
  struct EventLoop* evLoop; // 当前工作线程独占的事件循环，即子 Reactor
};

// MODIFIED: restored WorkThread fields and declarations.
int workerThreadInit(struct WorkThread* thread, int index);
void workerThreadRun(struct WorkThread* thread);
