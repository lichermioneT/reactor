#include "TcpServer.h"

#include "TcpConnection.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct Listener* listenerInit(unsigned short port)
{
// 1.开空间，存放监听的套接字和监听的端口信息
  struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
  if(listener == NULL)
  {
    perror("malloc");
    return NULL;
  }

// 2.创建监听的套接字
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd < 0)
  {
    perror("socket");
    free(listener);
    return NULL;
  }

// 3.设置端口复用
  int opt = 1;
  if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    perror("setsockopt");
    close(listenfd);
    free(listener);
    return NULL;
  }

// 4.将监听和端口信息绑定了
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if(bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    close(listenfd);
    free(listener);
    return NULL;
  }

// 5.将状态设置为监听的状态，监听的队列为128
  if(listen(listenfd, 128) < 0)
  {
    perror("listen");
    close(listenfd);
    free(listener);
    return NULL;
  }

  listener->lfd = listenfd;
  listener->port = port;

// 6.监听的文件描述符和监听的端口返回
  return listener;
}

int acceptConnection(void* arg)
{
// 1.数据类型的转换
  struct TcpServer* tcpServer = (struct TcpServer*)arg;
  if(tcpServer == NULL || tcpServer->listener == NULL)
  {
    return -1;
  }

// 2.接收新的链接信息
  int cfd = accept(tcpServer->listener->lfd, NULL, NULL);
  if(cfd < 0)
  {
    perror("accept");
    return -1;
  }

// 3.线程池里面拿一个反应堆的模型
  struct EventLoop* evLoop = takeWorkerEventLoop(tcpServer->threadPool);
  if(evLoop == NULL)
  {
    close(cfd);
    return -1;
  }

// 4.为这个文件描述符，注册读写事件和对应的空间信息
  tcpConnectionInit(cfd, evLoop);
  return 0;
}

struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
// 1.开空间的
  struct TcpServer* tcpServer = (struct TcpServer*)malloc(sizeof(struct TcpServer));
  if(tcpServer == NULL)
  {
    perror("malloc");
    return NULL;
  }

// 2.线程池，主线程的反应堆，监听套接字的信息
  tcpServer->threadNum = threadNum;
  tcpServer->listener = listenerInit(port);
  tcpServer->mainLoop = EventLoopInit();
  tcpServer->threadPool = NULL;

  if(tcpServer->listener == NULL || tcpServer->mainLoop == NULL)
  {
    free(tcpServer);
    return NULL;
  }

// 3.初始化，监听的信息，主线程的反应堆，线程池
  tcpServer->threadPool = threadPollInit(tcpServer->mainLoop, threadNum);
  if(tcpServer->threadPool == NULL)
  {
    close(tcpServer->listener->lfd);
    free(tcpServer->listener);
    free(tcpServer);
    return NULL;
  }

// 4.返回给调用者
  return tcpServer;
}

void TcpServerRun(struct TcpServer* server)
{
  if(server == NULL || server->threadPool == NULL || server->listener == NULL ||
     server->mainLoop == NULL)
  {
    return;
  }

// 1.运行线程池
  threadPoolRun(server->threadPool);

// 2.监听的文件描述符封装，监听文件描述符的读事件
// 2.1得到新的文件描述符 2.2拿一个线程池的反应堆，2.3为这个文件描述符注册读写事件和空间信息的。 2.4添加到反应堆模型里面去的
  struct Channel* channel = channelInit(server->listener->lfd, ReadEvent, acceptConnection, NULL, NULL, server);
  if(channel == NULL)
  {
    return;
  }

  // MODIFIED: listener fd registration is now guarded against allocation failure.
// 3.添加到任务队列里面
  eventLoopAddTask(server->mainLoop, channel, ADD);

// 4.主线程进行运行
  eventLoopRun(server->mainLoop);
}
