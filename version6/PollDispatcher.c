#include "Dispatcher.h"
#include <unistd.h>
#include "Channel.h"
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>

#define Max 1024

struct PollData 
{
  int maxfd;
  struct pollfd fds[Max];
};

  // 初始化
static  void* pollinit();
  // 添加
static int pollAdd(struct Channel* channel, struct EventLoop* evLoop);
  // 删除
static int pollRemove(struct Channel* channel, struct EventLoop* evLoop);
  // 修改
static int pollModify(struct Channel* channel, struct EventLoop* evLoop);
  // 事件监测
static int pollDispatcher(struct EventLoop* evLoop, int timeout);
  // 清除数据
static int pollClear(struct EventLoop* evLoop);

struct Dispatcher PollDispatcher = 
{
  pollinit,
  pollAdd,
  pollRemove,
  pollModify,
  pollDispatcher,
  pollClear
};

// 初始化
static void* pollinit()
{
  struct PollData* data = (struct PollData*)malloc(sizeof(struct PollData)); 
  data->maxfd = 0;
  
  for(int i = 0; i < Max; ++i)
  {
    data->fds[i].fd = -1;
    data->fds[i].events = 0;
    data->fds[i].revents = 0;
  }
  
  return data;
}
  // 添加
static int pollAdd(struct Channel* channel, struct EventLoop* evLoop)
{
  struct PollData* data = (struct PollData*)evLoop->dispatcherData;
  int events = 0;
  if(channel->events & ReadEvent)
  {
    events |= POLLIN;
  }
  if(channel->events & WriteEvent)
  {
    events |= POLLOUT;
  }
  
  int i = 0;
  for(i = 0; i < Max; ++i)
  {
    if(data->fds[i].fd == -1)
    {
        data->fds[i].events = events;
        data->fds[i].fd = channel->fd;
        data->maxfd = data->maxfd > i ? data->maxfd : i;
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
static int epollRemove(struct Channel* channel, struct EventLoop* evLoop)
{
  struct PollData* data = (struct PollData*)evLoop->dispatcherData;
  
  int i = 0;
  for(i = 0; i < Max; ++i)
  {
    if(data->fds[i].fd == channel->fd)
    {
        data->fds[i].events = 0;
        data->fds[i].revents = 0;
        data->fds[i].fd = -1;
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
static int pollModify(struct Channel* channel, struct EventLoop* evLoop)
{
  struct PollData* data = (struct PollData*)evLoop->dispatcherData;
  int events = 0;
  if(channel->events & ReadEvent)
  {
    events |= POLLIN;
  }
  if(channel->events & WriteEvent)
  {
    events |= POLLOUT;
  }
  
  int i = 0;
  for(i = 0; i < Max; ++i)
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
static int pollDispatcher(struct EventLoop* evLoop, int timeout)
{
  struct PollData* data = (struct PollData*)evLoop->dispatcherData;
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
    if(data->fds[i].events & POLLOUT)
    {

    }
  }
  
  return 0;
}
  // 清除数据
static int epollClear(struct EventLoop* evLoop)
{
  struct PollData* data = (struct PollData*)evLoop->dispatcherData;
  free(data);
  return 0;
}
