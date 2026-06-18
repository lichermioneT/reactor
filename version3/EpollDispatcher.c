#include "Dispatcher.h"
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

// Dispatcher 是底层事件检测机制的抽象接口；
// EventLoop 是上层事件循环对象，它通过 Dispatcher 去管理和分发 fd 事件

// 静态函数的声明
// 1.初始化数据块的。epoll,poll,select。返回值兼容三种数据类型的。
static void* epollinit(); 
// 2.添加文件描述符。(文件描述符在channel里面封装了的。)
static int epolladd(struct Channel* channel, struct EventLoop* evloop); 
// 3.删除节点
static int epollremove(struct Channel* channel, struct EventLoop* evloop); 
// 4.修改的
static int epollmodify(struct Channel* channel, struct EventLoop* evloop); 
// 5.事件的检测
static int epolldispatch(struct EventLoop* evloop, int timeout); 
 //6.关闭文件描述符
static int epollclear(struct EventLoop* evloop); 

// 定义一个局部函数
static int epollCtl(struct Channel* channel, struct EventLoop* evloop, int op);

// Dispatcher分发起
struct Dispatcher EpollDispatcher =
{
  epollinit,
  epolladd,
  epollremove,
  epollmodify,
  epolldispatch,
  epollclear,
};

struct EpollData
{
  int epfd; // epoll的句柄
  struct epoll_event* events; // epoll需要的结构体
};

#define Max 520

// 初始化epoll需要的数据
static void* epollinit()
{
  // 1.开辟堆空间，存放epoll需要的数据
  struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));

  // 2.创建epoll树
  data->epfd = epoll_create(10);
  if(data->epfd == -1)
  {
    perror("epoll_create");
    exit(0);
  }
  
  // 3.开辟空间epoll需要的数组
  data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));
  
  // 4.epoll句柄和对应的数组返回的。
  return data;
}

static int epollCtl(struct Channel* channel, struct EventLoop* evloop, int op)
{
  // 1.EventLoop里面的dispatcherdata里面拿数据的。
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata;
  
  // 2.创建一个额外的事件，是否需要检查读写。
  struct epoll_event ev;
  ev.data.fd = channel->fd;
  
  int events = 0;
  if(channel->events & ReadEvent)
  {
    events |= EPOLLIN;
  }
  if(channel->events & WriteEvent)
  {
    events |= EPOLLOUT;
  }
  
  ev.events = events;
  int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);
  
  return ret;
}

// 1.添加
static int epolladd(struct Channel* channel, struct EventLoop* evloop)
{
  int ret = epollCtl(channel, evloop, EPOLL_CTL_ADD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_ADD");
    exit(0);
  }
  return ret;
}

// 2.删除
static int epollremove(struct Channel* channel, struct EventLoop* evloop)
{
  int ret = epollCtl(channel, evloop, EPOLL_CTL_DEL);
  if(ret == -1)
  {
    perror("EPOLL_CTL_DEL");
    exit(0);
  }
  return ret;
}

// 3.修改
static int epollmodify(struct Channel* channel, struct EventLoop* evloop)
{
  int ret = epollCtl(channel, evloop, EPOLL_CTL_MOD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_MOD");
    exit(0);
  }
  return ret;
}

// 4.事件的检测
static int epolldispatch(struct EventLoop* evloop, int timeout)
{
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata;
  
  int count = epoll_wait(data->epfd, data->events, Max, timeout * 1000);
  for(int i = 0; i < count; i++)
  {
    int events = data->events[i].events;
    int fd = data->events[i].data.fd;

    if(events & EPOLLERR || events &  EPOLLHUP)
    {
      continue;
    }

    // 事件的激活
    if(events & EPOLLIN)
    {
      eventActivate(evloop, fd, ReadEvent);
    }
    if(events & EPOLLOUT)
    {
      eventActivate(evloop, fd, WriteEvent);
    }
  }

  return 0;
}
 //6.关闭文件描述符
static int epollclear(struct EventLoop* evloop)
{
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata;
  free(data->events);
  close(data->epfd);
  free(data);
  return 0;
}
