#include "Dispatcher.h"
#include "EventLoop.h"
#include "Channel.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

static void* epollinit();
static int epolladd(struct Channel* channel, struct EventLoop* evloop);
static int epollremove(struct Channel* channel, struct EventLoop* evloop);
static int epollmodify(struct Channel* channel, struct EventLoop* evloop);
static int epolldispatch(struct EventLoop* evloop, int timeout);
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
  struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
  if(data == NULL)
  {
    perror("malloc");
    return NULL;
  }

  data->epfd = epoll_create(10);
  if(data->epfd == -1)
  {
    perror("epoll_create");
    free(data);
    return NULL;
  }

  data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));
  if(data->events == NULL)
  {
    perror("calloc");
    close(data->epfd);
    free(data);
    return NULL;
  }

  return data;
}

static int epolldispatch(struct EventLoop* evloop, int timeout)
{
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata;
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

  for(int i = 0; i < count; i++)
  {
    int events = data->events[i].events;
    int fd = data->events[i].data.fd;

    if((events & EPOLLERR) || (events & EPOLLHUP))
    {
      continue;
    }

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
  int ret = epollCtl(channel, evloop, EPOLL_CTL_ADD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_ADD");
  }
  return ret;
}

static int epollremove(struct Channel* channel, struct EventLoop* evloop)
{
  int ret = epollCtl(channel, evloop, EPOLL_CTL_DEL);
  if(ret == -1)
  {
    perror("EPOLL_CTL_DEL");
  }

  if(channel->destoryCallback != NULL)
  {
    channel->destoryCallback(channel->arg);
  }
  return ret;
}

static int epollmodify(struct Channel* channel, struct EventLoop* evloop)
{
  int ret = epollCtl(channel, evloop, EPOLL_CTL_MOD);
  if(ret == -1)
  {
    perror("EPOLL_CTL_MOD");
  }
  return ret;
}

static int epollclear(struct EventLoop* evloop)
{
  struct EpollData* data = (struct EpollData*)evloop->dispatcherdata;
  if(data != NULL)
  {
    free(data->events);
    close(data->epfd);
    free(data);
  }
  return 0;
}
