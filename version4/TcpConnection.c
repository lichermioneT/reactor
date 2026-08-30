#include "TcpConnection.h"

#include <stdio.h>
#include <stdlib.h>

static int processRead(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;
  if(conn == NULL)
  {
    return -1;
  }

  int count = bufferSocketRead(conn->readBuf, conn->channel->fd);
  if(count > 0)
  {
    int socket = conn->channel->fd;
    bool flag = parseHttpRequest(conn->request, conn->readBuf, conn->response,
                                 conn->writeBuf, socket);
    if(!flag)
    {
      const char* errMsg =
          "HTTP/1.1 400 Bad Request\r\nContent-length: 0\r\n\r\n";
      bufferAppendString(conn->writeBuf, errMsg);
    }

#ifdef MSG_SEND_AUTO
    writeEventEnable(conn->channel, true);
    eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
#else
    bufferSendData(conn->writeBuf, socket);
    eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
  }
  else
  {
    eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
  }

  return 0;
}

static int processWrite(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;
  if(conn == NULL)
  {
    return -1;
  }

  // MODIFIED: restored count declaration that had been swallowed by a comment.
  int count = bufferSendData(conn->writeBuf, conn->channel->fd);
  if(count >= 0 && bufferReadableSize(conn->writeBuf) == 0)
  {
    writeEventEnable(conn->channel, false);
    eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
    eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
  }

  return 0;
}

int tcpConnectionDestory(void* arg)
{
  struct TcpConnection* conn = (struct TcpConnection*)arg;
  if(conn == NULL)
  {
    return 0;
  }

  if(conn->channel != NULL)
  {
    destroyChannel(conn->evLoop, conn->channel);
    conn->channel = NULL;
  }

  bufferDestory(conn->readBuf);
  bufferDestory(conn->writeBuf);
  httpRequestDestory(conn->request);
  httpResponeDestory(conn->response);
  free(conn);
  return 0;
}

struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evloop)
{
  if(fd < 0 || evloop == NULL)
  {
    return NULL;
  }

// 1.开空间
  struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
  if(conn == NULL)
  {
    perror("malloc");
    return NULL;
  }

// 2.初始化数据，反应堆，监听的文件描述符封装成chanenl, 读写数据的内存，请求和响应的
  conn->evLoop = evloop;
  conn->channel = channelInit(fd, ReadEvent, processRead, processWrite, tcpConnectionDestory, conn);
  conn->readBuf = bufferInit(10240);
  conn->writeBuf = bufferInit(10240);
  
  conn->request = httpRequestInit();
  conn->response = httResponseInit();

  snprintf(conn->name, sizeof(conn->name), "Connect-%d", fd);

  if(conn->channel == NULL || conn->readBuf == NULL || conn->writeBuf == NULL ||
     conn->request == NULL || conn->response == NULL)
  {
    tcpConnectionDestory(conn);
    return NULL;
  }

// 3.文件描述符添加到反应堆模型
  eventLoopAddTask(evloop, conn->channel, ADD);
  return conn;
}