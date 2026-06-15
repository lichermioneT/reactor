#include "dispatcher.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#define Max 520

struct EpollData
{
  int epfd;
  struct epoll_event* events;
};

// 声明
static void* epollinit();
// 添加
static int epolladd(struct Channel* channel, struct eventloop* evloop);
// 删除
static int epollremove(struct Channel* channel, struct eventloop* evloop);
// 修改
static int epollmodify(struct Channel* channel, struct eventloop* evloop);
// 事件监测
static int epolldispatcher(struct eventloop* evloop, int timeout);
// 资源释放 
static int epoollclear(struct eventloop* evloop);
static int epollctl(struct Channel* channel, struct eventloop* evloop, int op);
// 事件监测

struct dispatcher epolldispatch = 
{
  epollinit,
  epolladd,
  epollremove,
  epollmodify,
  epolldispatcher,
  epoollclear
};

// 实现
static void* epollinit()
{
// 1.创建epoll实例
  struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
  data->epfd = epoll_create(10);
  if(data->epfd == 1)
  {
    perror("epoll_create");
    exit(0);
  }
// 2.创建epoll需要的额外数组
  data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));

// 3.返回给调用者
  return data;
}
static int epollctl(struct Channel* channel, struct eventloop* evloop, int op)
{
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata; 
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
  // 添加
static int epolladd(struct Channel* channel, struct eventloop* evloop)
{
  int ret = epollctl(channel,evloop, EPOLL_CTL_ADD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_ADD");
    exit(0);
  }
  return ret;
}

// 删除
static int epollremove(struct Channel* channel, struct eventloop* evloop)
{
  int ret = epollctl(channel,evloop, EPOLL_CTL_DEL);
  if(ret == -1)
  {
    perror("EPOLL_CTL_DEL");
    exit(0);
  }
  return ret;
}
// 修改
static int epollmodify(struct Channel* channel, struct eventloop* evloop)
{
  int ret = epollctl(channel,evloop, EPOLL_CTL_MOD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_MOD");
    exit(0);
  }
  return ret;
}
// 事件监测
static int epolldispatcher(struct eventloop* evloop, int timeout)
{
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata; 
  int count = epoll_wait(data->epfd, data->events, Max, timeout * 1000);
  for(int i = 0; i < count;  ++i)
  {
    int events = data->events[i].events;
    int fd = data->events[i].data.fd;
    if(events & EPOLLERR & events &EPOLLHUP)
    {
      //
      continue;
    }

    if(events & EPOLLIN)
    {

    }

    if(events & EPOLLOUT)
    {

    }
  }

  return 0;
}
// 资源释放 
static int epoollclear(struct eventloop* evloop)
{
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata; 
  free(data->events);
  close(data->epfd);
  free(data);
  
  return 0;
}
