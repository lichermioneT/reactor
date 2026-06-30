#include "Channel.h"

#include <stdio.h>
#include <stdlib.h>

struct Channel* channelInit(int fd, int events, 
                            handleFunc reaFunc,
                            handleFunc writeFunc, 
                            handleFunc destoryCallback,
                            void* arg)
{
// 1.开空间
  struct Channel* data = (struct Channel*)malloc(sizeof(struct Channel));
  if(data == NULL)
  {
    perror("malloc");
    return NULL;
  }

// 2.初始化数据
  data->fd = fd;
  data->events = events; // 判断是否是读写的事件
  data->readCallback = reaFunc;
  data->writeCallback = writeFunc;
  data->destoryCallback = destoryCallback;
  data->arg = arg;

// 3.返回给调用者的
  return data;
}

void writeEventEnable(struct Channel* channel, bool flag)
{
  if(channel == NULL)
  {
    return;
  }

  if(flag)
  {
// 1. 开启写事件
    channel->events |= WriteEvent; 
  }
  else
  {
// 2. 关闭写事件
    channel->events &= ~WriteEvent; 
  }
}

// 判断是否可以进行监听的。
bool isWriteEventEnable(struct Channel* channel)
{
  return channel != NULL && (channel->events & WriteEvent); // 是否监听写事件的
}