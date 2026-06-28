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
  int threadNum;
  struct ThreadPool* threadPool;
  struct EventLoop* mainLoop;
  struct Listener* listener;
};

// MODIFIED: restored server declarations and acceptConnection prototype.
struct Listener* listenerInit(unsigned short port);
struct TcpServer* tcpServerInit(unsigned short port, int threadNum);
void TcpServerRun(struct TcpServer* server);
int acceptConnection(void* arg);
