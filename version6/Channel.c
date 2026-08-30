#include "Channel.h"
#include <stdlib.h>
#include <stdio.h>

struct Channel* channelInit(int fd, int events, handleFunc readFunc, handleFunc writeFunc, void* arg)
{
// 1.开空间
  struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
  if(channel == NULL)
  {
    perror("malloc");
    return  NULL;
  }

// 2.初始化
  channel->fd = fd;
  channel->events = events;
  channel->readCallback = readFunc;
  channel->writeCallback = writeFunc;
  channel->arg = arg;

// 3.返回值
  return channel;
}

void writeEventEnable(struct Channel* channel, bool flag)
{
  if(flag)
  {
    channel->events |= WriteEvent; // 追加写事件
  }
  else 
  {
    channel->events = channel->events & ~WriteEvent; // 删除写事件
  }
}

bool isWriteEventEnable(struct Channel* channel)
{
  return  channel->events & WriteEvent; // 判断写事件是否开启的
}
