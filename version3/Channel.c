#include "Channel.h"
#include <stdio.h>
#include <stdlib.h>

// 1.开空间，初始化数据的
struct Channel* channelInit(int fd, int events, handleFunc reaFunc, handleFunc writeFunc, void* arg)
{
  // 1.申请内存
  struct Channel* data = (struct Channel*)malloc(sizeof(struct Channel));
  if(data == NULL)
  {
    perror("malloc");
    return NULL;
  }
  
  // 2.内存写数据
  data->fd = fd;
  data->events = events;
  data->readCallback = reaFunc;
  data->writeCallback = writeFunc;
  // 读写函数需要的参数
  data->arg = arg;
  
  // 3.返回给调用者,返回一个channel的指针。
  return data;
}

// 2.是否需要检查写的事件
void writeEventEnable(struct Channel* channel, bool flag)
{
  if(flag)
  {
     // 1.flag真的话，就获取写的事件
     channel->events |= WriteEvent; //2-->的二进制 10
  }
  else 
  {
    // 2.删除写的事件
    channel->events = channel->events & ~WriteEvent;
  }
}

// 3.是否需要检查文件描述的写事件
bool isWriteEventEnable(struct Channel* channel)
{
  // 判断是否需要写
  return channel->events & WriteEvent;
}

// channel->events |= WriteEvent;      // 开启写事件
// channel->events &= ~WriteEvent;     // 关闭写事件
// channel->events & WriteEvent;       // 判断写事件
