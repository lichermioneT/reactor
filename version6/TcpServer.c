#include "TcpConnection.h"
#include "TcpServer.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

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
    close(listenfd); 
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
    close(listenfd); 
    return NULL;
  }

  // 4.监听
  if(listen(listenfd, 128) < 0)
  {
    perror("listen");
    close(listenfd); 
    return NULL;
  }

  listener->lfd =listenfd;
  listener->port = port;

  return listener;
}

struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
  struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
  tcp->listener = listenerInit(port);  
  tcp->mainLoop = eventLoopInit();
  tcp->threadNum = threadNum;
  tcp->threadPool = threadPoolInit(tcp->mainLoop, threadNum);

  return tcp;
}

int acceptCoonection(void* arg)
{
  struct TcpServer* server = (struct TcpServer*)arg;
  // 建立链接
  int cfd = accept(server->listener->lfd, NULL, NULL);
  
  // 放到子线程去处理
  struct EventLoop* evLoop = takeWorkerEventLoop(server->threadPool);
  tcpConnectionInit(cfd, evLoop);  
}

void tcpServerRun(struct TcpServer* server)
{
  // 启动线程池
  threadPoolRun(server->threadPool);
  
  struct Channel* channel = channelInit(server->listener->lfd, ReadEvent, acceptCoonection, NULL, server);

  // 添加检测的任务 
  eventLoopAddTask(server->mainLoop, channel, ADD);
  // 启动反应堆模型
  eventLoopRun(server->mainLoop);
}
