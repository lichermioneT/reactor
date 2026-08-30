#include "TcpConnection.h"
#include <stdio.h>
#include <stdlib.h>

int processRead(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;  
  // 接收数据
  int count = bufferSocketRead(conn->readBuf, conn->channel->fd);
  if(count > 0)
  {
    // 接收到了， 解析http
  }
  else 
  {
    // 断开连接
  }
}

struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evLoop)
{
  struct TcpConnection* tcpConnect = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
  tcpConnect->evLoop = evLoop;
  tcpConnect->readBuf = bufferInit(10240);
  tcpConnect->wirteBuf = bufferInit(10240);
  
  sprintf(tcpConnect->name, "Connect-%d", fd);
  tcpConnect->channel = channelInit(fd, ReadEvent, processRead, NULL, tcpConnect);
  eventLoopAddTask(evLoop,tcpConnect->channel, ADD);

  return tcpConnect;
}
