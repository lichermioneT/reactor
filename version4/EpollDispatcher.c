#include "Dispatcher.h"
#include "EventLoop.h"
#include "Channel.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

/*
EpollDispatcher模块总结
1.初始化函数，创建epoll树的句柄，和epoll需要的额外数组
2.事件分发函数，监听事件返回， 激活读写的事件。 监听套接字注册的函数就是事件的分发
3.add:channel
4.remove:channel
5.modify:channel
6.clear:data(辅助的空间)
*/

static void* epollinit();
static int epolldispatch(struct EventLoop* evloop, int timeout);

static int epolladd(struct Channel* channel, struct EventLoop* evloop);
static int epollremove(struct Channel* channel, struct EventLoop* evloop);
static int epollmodify(struct Channel* channel, struct EventLoop* evloop);

static int epollclear(struct EventLoop* evloop);
static int epollCtl(struct Channel* channel, struct EventLoop* evloop, int op);

// MODIFIED: restored dispatcher object declaration.
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
  int epfd;
  struct epoll_event* events;
};

#define Max 520

static void* epollinit()
{
// 1.开空间，存放epoll句柄和辅助数组。
  struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
  if(data == NULL)
  {
    perror("malloc");
    return NULL;
  }

// 2.1创建epoll树
  data->epfd = epoll_create(10);
  if(data->epfd == -1)
  {
    perror("epoll_create");
    free(data);
    return NULL;
  }
// 2.2初始化Max大小各数组的
  data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));
  if(data->events == NULL)
  {
    perror("calloc");
    close(data->epfd);
    free(data);
    return NULL;
  }

// 3.返还给调用者
  return data;
}

static int epolldispatch(struct EventLoop* evloop, int timeout)
{
// 1.从EventLoop里面拿来辅助的数组。
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata;

// 2.开始进行监听了
  int count = epoll_wait(data->epfd, data->events, Max, timeout * 1000);
  if(count == -1)
  {
    if(errno == EINTR)
    {
      return 0;
    }
    perror("epoll_wait");
    return -1;
  }

// 3.遍历已经成功检查的事件
  for(int i = 0; i < count; i++)
  {
    int events = data->events[i].events;
    int fd = data->events[i].data.fd;

    // 3.1出错的不管
    if((events & EPOLLERR) || (events & EPOLLHUP))
    {
      continue;
    }
    
    // 3.2激活读事件
    if(events & EPOLLIN)
    {
      eventActivate(evloop, fd, ReadEvent);
    }

    // 3.3激活写事件
    if(events & EPOLLOUT)
    {
      eventActivate(evloop, fd, WriteEvent);
    }
  }

  return 0;
}

static int epollCtl(struct Channel* channel, struct EventLoop* evloop, int op)
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
  return epoll_ctl(data->epfd, op, channel->fd, &ev);
}


static int epolladd(struct Channel* channel, struct EventLoop* evloop)
{
// 1.先判断channel->fd是否需要：读，写
// 2.添加epoll数组里面去。
  int ret = epollCtl(channel, evloop, EPOLL_CTL_ADD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_ADD");
  }
  return ret;
}

static int epollremove(struct Channel* channel, struct EventLoop* evloop)
{
// 1.先判断channel->fd是否需要：读，写
// 2.删除epoll数组里面去。
  int ret = epollCtl(channel, evloop, EPOLL_CTL_DEL);
  if(ret == -1)
  {
    perror("EPOLL_CTL_DEL");
  }

// 3.清理函数？？？？？
  if(channel->destoryCallback != NULL)
  {
    channel->destoryCallback(channel->arg);
  }
  return ret;
}

static int epollmodify(struct Channel* channel, struct EventLoop* evloop)
{
// 1.先判断channel->fd是否需要：读，写
// 2.修改epoll数组里面去。
  int ret = epollCtl(channel, evloop, EPOLL_CTL_MOD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_MOD");
  }
  return ret;
}

static int epollclear(struct EventLoop* evloop)
{
// 1.删除需要辅助的数组
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata;
  if(data != NULL)
  {
    free(data->events);
    close(data->epfd);
    free(data);
  }
  return 0;
}
