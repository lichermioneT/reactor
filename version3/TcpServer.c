#include "TcpServer.h"
#include "TcpConnection.h"
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>

// 监听初始化四部曲1.socket, 2.setsockopt, 3.bind, 4.listen
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

// tcp初始化1.空间， 2.需要监听的端口和监听的队列。3.反应堆模型。4.线程池。
struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
  struct TcpServer* tcpServer = (struct TcpServer*)malloc(sizeof(struct TcpServer)); // 开空间
  tcpServer->listener = listenerInit(port); // 需要监听的端口
  tcpServer->mainLoop = EventLoopInit(); // 主线程的反应堆 示例的初始化。
  tcpServer->threadNum = threadNum;   // 线程池的线程个数
  tcpServer->threadPool = threadPollInit(tcpServer->mainLoop, threadNum); // 线程池

  return tcpServer;
}

int acceptConnection(void* arg)
{
  struct TcpServer* tcpServer = (struct TcpServer*)arg;
  
  int cfd = accept(tcpServer->listener->lfd, NULL, NULL);

  // 从线程池里面取一个子线程的反应堆模型，处理cfd
  struct EventLoop* evLoop = takeWorkerEventLoop(tcpServer->threadPool);

  tcpConnectionInit(cfd, evLoop); 

  return 0;
}

void TcpServerRun(struct TcpServer* server)
{
// 启动线程池
  threadPoolRun(server->threadPool);
  
// 初始化一个channel示例,这里就是监听的文件描述符
  struct Channel* channel = channelInit(server->listener->lfd, ReadEvent, acceptConnection, NULL, server);

  // 添加检查任务,主线程的反应堆模型。
  // 添加到反应堆里面，主线程的反应堆模型
  // channel就是封装的文件描述符
// 任务队列，对检测集合应该如何操作
  eventLoopAddTask(server->mainLoop, channel, ADD);
// 启动反应堆模型
  eventLoopRun(server->mainLoop);
}
