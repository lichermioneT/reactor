#include "TcpServer.h"
#include "TcpConnection.h"
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>

struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
  struct TcpServer* tcpServer = (struct TcpServer*)malloc(sizeof(struct TcpServer));
  tcpServer->listener = listenerInit(port);
  tcpServer->mainLoop = EventLoopInit();
  tcpServer->threadNum = threadNum;
  tcpServer->threadPool = threadPollInit(tcpServer->mainLoop, threadNum);

  return tcpServer;
}

struct Listener* listenerInit(unsigned short port)
{
  struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
  
   // 1.创建监听的fd
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd <  0)
  {
    perror("socket");
    return NULL;
  }

  // 2.设置端口复用
  int opt = 1;
  if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0)
  {
    perror("setsockopt");
    return NULL;
  }
  
  // 3.绑定
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意网卡地址
  addr.sin_port = htons(port);

  if(bind(listenfd, (struct sockaddr*)&addr, sizeof addr) < 0)
  {
    perror("bind");
    return NULL;
  }

  // 4.监听
  if(listen(listenfd, 128) < 0)
  {
    perror("listen");
    return NULL;
  }
  
  listener->lfd = listenfd;
  listener->port = port;

  return listener;
}

int acceptConnection(void* arg)
{
  struct TcpServer* tcpServer = (struct TcpServer*)arg;
  
  int cfd = accept(tcpServer->listener->lfd, NULL, NULL);

  // 从线程池里面取一个子线程的反应堆模型，处理cfd
  struct EventLoop* evLoop = takeWorkerEventLoop(tcpServer->threadPool);

  tcpConnectionInit(cfd, evLoop); 
}

void TcpServerRun(struct TcpServer* server)
{
  // 启动线程池
  threadPoolRun(server->threadPool);
  
  // 初始化一个channel示例
  struct Channel* channel = channelInit(server->listener->lfd, ReadEvent, acceptConnection, NULL, server);

  // 添加检查任务
  eventLoopAddTask(server->mainLoop, channel, ADD);
  // 启动反应堆模型
  eventLoopRun(server->mainLoop);
}
