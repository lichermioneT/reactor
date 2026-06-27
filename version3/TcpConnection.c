#include "TcpConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <stdlib.h>
#include <stdio.h>

int processRead(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;
  
  int connt = bufferSocketRead(conn->readBuf, conn->channel->fd);
  if(connt > 0)
  {
    // 接收到了http请求，解析http请求 
    int socket = conn->channel->fd; 
#ifdef MSG_SEND_AUTO
    writeEventEnable(conn->channel, true);
    eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
#endif
    bool flag =  parseHttpRequest(conn->request, conn->readBuf, conn->response, conn->writeBuf, socket);
    if(!flag)
    {
      // 解析失败
      char* errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
      bufferAppendString(conn->writeBuf, errMsg);
    }
  }

#ifndef MSG_SEND_AUTO
  //断开链接
  eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
  return 0;
}

int processWrite(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;
  // 取数据，发送数据
  int count = bufferSendData(conn->writeBuf, conn->channel->fd);
  if(count > 0)
  {
    // 判断是否完全发送去取了
    if(bufferReadableSize(conn->writeBuf) == 0)
    {
      // 1.不再检查写事件，修改channel
      writeEventEnable(conn->channel, false);
      // 2.修改dispatcher集合
      eventLoopAddTask(conn->evLoop, conn->channel,MODIFY);
      // 3.删除这个节点信息
      eventLoopAddTask(conn->evLoop, conn->channel,DELETE);
    }
  }
  return 0;
}

struct TcpConnection* tcpConnectionInit(int fd,  struct EventLoop* evloop)
{
  // 1.申请堆内存
  struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
  conn->evLoop = evloop;
  conn->readBuf  = bufferInit(10240);
  conn->writeBuf  = bufferInit(10240);

  // http 
  conn->request = httpRequestInit();
  conn->response  = httResponseInit();
  sprintf(conn->name, "Connect-%d", fd);

  // 初始化的channel
  conn->channel = channelInit(fd, ReadEvent, processRead, processWrite, tcpConnectionDestory,conn); 

  // 添加到任务队列里面去的
  eventLoopAddTask(evloop, conn->channel, ADD);

  return conn;
}

int tcpConnectionDestory(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;
  if(conn !=  NULL)
  {
    if(conn->readBuf  && bufferReadableSize(conn->readBuf) == 0 &&
       conn->writeBuf && bufferReadableSize(conn->writeBuf) == 0)
    {
      destroyChannel(conn->evLoop, conn->channel);
      bufferDestory(conn->readBuf);
      bufferDestory(conn->writeBuf);
      httpRequestDestory(conn->request);
      httpResponeDestory(conn->response);
      free(conn);
    }
  }

  return 0;
}
