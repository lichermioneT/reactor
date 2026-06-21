#include "TcpConnection.h"
#include <stdlib.h>
#include <stdio.h>

int processRead(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;
  
  int connt = bufferSocketRead(conn->readBuf, conn->channel->fd);
  if(connt > 0)
  {
    // 接收到了http请求，解析http请求 
  }
  else 
  {
    // 断开连接
  }

  return connt;
}


struct TcpConnection* tcpConnectionInit(int fd,  struct EventLoop* evloop)
{
  struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
  conn->evLoop = evloop;
  conn->readBuf  = bufferInit(10240);
  conn->writeBuf  = bufferInit(10240);
  sprintf(conn->name, "Connect-%d", fd);
  conn->channel = channelInit(fd, ReadEvent, processRead, NULL, conn); 
  eventLoopAddTask(evloop, conn->channel, ADD);

  return conn;
}
