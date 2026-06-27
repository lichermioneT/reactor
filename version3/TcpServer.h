#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

struct Listener
{
  int lfd;
  unsigned short port;
};

struct TcpServer 
{
  int threadNum;                 // 线程池的个数
  struct EventLoop* mainLoop;    // 反应堆模型
  struct ThreadPool* threadPool; // 线程池
  struct Listener* listener;     // 监听的文件描述符和端口
};

// 初始化监听
// 监听的端口和用于监听的文件描述符
struct Listener* listenerInit(unsigned short port);

// 初始化1.监听的信息
// 2.主线程的反应堆和线程池
struct TcpServer* tcpServerInit(unsigned short port, int threadNum);
// 启动服务器
void TcpServerRun(struct TcpServer* server);
