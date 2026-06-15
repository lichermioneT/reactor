#include "dispatcher.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>

#define Max 1024

struct pollData
{
  int maxfd;
  struct pollfd fds[Max];
};


// 声明
static void* pollinit();
// 添加
static int polladd(struct Channel* channel, struct eventloop* evloop);
// 删除
static int pollremove(struct Channel* channel, struct eventloop* evloop);
// 修改
static int pollmodify(struct Channel* channel, struct eventloop* evloop);
// 事件监测
static int polldispatcher(struct eventloop* evloop, int timeout);
// 资源释放 
static int poollclear(struct eventloop* evloop);
static int pollctl(struct Channel* channel, struct eventloop* evloop, int op);
// 事件监测

struct dispatcher polldispatch = 
{
  pollinit,
  polladd,
  pollremove,
  pollmodify,
  polldispatcher,
  poollclear
};

// 实现
static void* pollinit()
{
// 1.创建poll实例
  struct pollData* data = (struct pollData*)malloc(sizeof(struct pollData));
  data->maxfd = 0;
  
  for(int i = 0; i < Max; ++i)
  {
    data->fds[i].fd = -1;
    data->fds[i].events = -1;
    data->fds[i].revents = -1;
  }
  
  return data;
}
  // 添加
static int epolladd(struct Channel* channel, struct eventloop* evloop)
{
  struct pollData* data = (struct pollData*)evloop->dispatcherdata; 
  
  int events = 0;
  if(channel->fd & ReadEvent)
  {
    events |= POLLIN;
  }
  if(channel->fd & WriteEvent)
  {
    events |= POLLOUT;
  }
  
  int i = 0;
  for(; i < Max; ++i)
  {
    if(data->fds[i].fd == -1)
    { 
      data->fds[i].events = events;
      data->fds[i].fd = channel->fd;
      data->maxfd = i > data->maxfd ? i : data->maxfd;
      break;
    }
  }

  if(i >= Max)
  {
    return -1;
  }
  
  return 0;
}

// 删除
static int pollremove(struct Channel* channel, struct eventloop* evloop)
{
  struct pollData* data = (struct pollData*)evloop->dispatcherdata; 
  int i = 0;
  for(; i < Max; ++i)
  {
    if(data->fds[i].fd == channel->fd)
    {
      data->fds[i].events = 0;
      data->fds[i].revents = 0;
      data->fds[i].fd = 0;
      break;

    }
  }

  if(i >= Max)
  {
    return -1;
  }

  return 0;
}
// 修改
static int pollmodify(struct Channel* channel, struct eventloop* evloop)
{
  struct pollData* data = (struct pollData*)evloop->dispatcherdata; 
  
  int events = 0;
  if(channel->fd & ReadEvent)
  {
    events |= POLLIN;
  }
  if(channel->fd & WriteEvent)
  {
    events |= POLLOUT;
  }
  
  int i = 0;
  for(; i < Max; ++i)
  {
    if(data->fds[i].fd == channel->fd)
    { 
      data->fds[i].events = events;
      break;
    }
  }

  if(i >= Max)
  {
    return -1;
  }
  
  return 0;

}
// 事件监测
static int polldispatcher(struct eventloop* evloop, int timeout)
{
  struct pollData* data = (struct pollData*)evloop->dispatcherdata; 
  int count = poll(data->fds, data->maxfd + 1, timeout * 1000);
  if(count == -1)
  {
    perror("poll");
    exit(0);
  }

  for(int i = 0; i <= data->maxfd; ++i)
  {
    if(data->fds[i].fd == -1)
    {
      continue;
    }

    if(data->fds[i].revents & POLLIN)
    {

    }
    if(data->fds[i].revents & POLLOUT)
    {

    }
  }

  return 0;
}
// 资源释放 
static int epoollclear(struct eventloop* evloop)
{
  struct pollData* data = (struct pollData*)evloop->dispatcherdata; 
  free(data);

  return 0;
}
