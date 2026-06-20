#include "Dispatcher.h"
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>

/*
polldispatch模块的总结
1.初始化函数，开空间,poll需要的额外数组，然然初始化到-1.
2.channel里面的文件描述符，是否需要添加到反应堆里面去的。
3.channel里面的文件描述符，从反应堆删除
4.channel里面的文件描述符，修改读书事件的
5.检查事件
6.删除刚才开辟的空间
*/

// Dispatcher 是底层事件检测机制的抽象接口；
// EventLoop 是上层事件循环对象，它通过 Dispatcher 去管理和分发 fd 事件

// 静态函数的声明
// 1.初始化数据块的。epoll,poll,select。返回值兼容三种数据类型的。
static void* pollinit(); 
// 2.添加文件描述符。(文件描述符在channel里面封装了的。)
static int polladd(struct Channel* channel, struct EventLoop* evloop); 
// 3.删除节点
static int pollremove(struct Channel* channel, struct EventLoop* evloop); 
// 4.修改的
static int pollmodify(struct Channel* channel, struct EventLoop* evloop); 
// 5.事件的检测
static int polldispatch(struct EventLoop* evloop, int timeout); 
 //6.关闭文件描述符
static int pollclear(struct EventLoop* evloop); 

// Dispatcher分发起
struct Dispatcher PollDispatcher =
{
  pollinit,
  polladd,
  pollremove,
  pollmodify,
  polldispatch,
  pollclear,
};

#define Max 1024
struct PollData
{
  int maxfd;
  struct pollfd fds[Max];
};


// 初始化epoll需要的数据
static void* epollinit()
{
  // 1.开辟堆空间，存放poll需要的数据
  struct PollData* data = (struct PollData*)malloc(sizeof(struct PollData));

  // 2.初始化数据块
  data->maxfd = 0;
  for(int i = 0; i < Max; ++i)
  {
    data->fds[i].fd = -1;
    data->fds[i].events = 0;
    data->fds[i].revents = 0;
  }
 
  // 3.数据块返还给调用者
  return data;
}

// 1.添加
static int polladd(struct Channel* channel, struct EventLoop* evloop)
{
  // 1.EventLoop里面的dispatcherdata里面拿数据的。
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  
  // 2.判断是否需要记录读写事件 
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
  for(; i < Max; ++i)
  {
    if(data->fds[i].fd == 1)
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

// 2.删除
static int pollremove(struct Channel* channel, struct EventLoop* evloop)
{
  // 1.EventLoop里面的dispatcherdata里面拿数据的。
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  
  // 2.找到文件描述符 
  int i = 0;
  for(; i < Max; ++i)
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

// 3.修改
static int epollmodify(struct Channel* channel, struct EventLoop* evloop)
{
  // 1.EventLoop里面的dispatcherdata里面拿数据的。
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  
  // 2.判断是否需要记录读写事件 
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

// 4.事件的检测
static int epolldispatch(struct EventLoop* evloop, int timeout)
{
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  
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

    // 激活读写事件的
    if(data->fds[i].revents * POLLIN)
    {
        eventActivate(evloop, data->fds[i].fd, ReadEvent);
    }
    if(data->fds[i].revents & POLLOUT)
    {
        eventActivate(evloop, data->fds[i].fd, WriteEvent);
    }

  }

  return 0;
}
 //6.关闭文件描述符
static int pollclear(struct EventLoop* evloop)
{
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  free(data);
  return 0;
}
