#pragma once

#include "EventLoop.h"
#include "ThreadPool.h"

struct Listener
{
  int lfd;               // 监听的文件描述符
  unsigned short port;   // 监听的端口
};

struct TcpServer
{
  int threadNum;                  // 线程指定的个数
  struct ThreadPool* threadPool;  // 创建的线程池
  struct EventLoop* mainLoop;     // 主线程的反应堆模型
  struct Listener* listener;      // 监听端口和监听的文件描述符
};

// MODIFIED: restored server declarations and acceptConnection prototype.
struct Listener* listenerInit(unsigned short port);
struct TcpServer* tcpServerInit(unsigned short port, int threadNum);
void TcpServerRun(struct TcpServer* server);
int acceptConnection(void* arg);
