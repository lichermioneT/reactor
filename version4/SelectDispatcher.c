#include "Dispatcher.h"
#include "EventLoop.h"
#include "Channel.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#define Max 1024

struct SelectData
{
  fd_set readSet;
  fd_set writeSet;
};

static void* selectinit();
static int selectadd(struct Channel* channel, struct EventLoop* evloop);
static int selectremove(struct Channel* channel, struct EventLoop* evloop);
static int selectmodify(struct Channel* channel, struct EventLoop* evloop);
static int selectdispatch(struct EventLoop* evloop, int timeout);
static int selectclear(struct EventLoop* evloop);
static void setFdSet(struct Channel* channel, struct SelectData* data);
static void clearFdSet(struct Channel* channel, struct SelectData* data);

// MODIFIED: restored select dispatcher declaration and fixed modify clearing.
struct Dispatcher SelectDispatcher =
{
  selectinit,
  selectadd,
  selectremove,
  selectmodify,
  selectdispatch,
  selectclear,
};

static void setFdSet(struct Channel* channel, struct SelectData* data)
{
  if(channel->events & ReadEvent)
  {
    FD_SET(channel->fd, &data->readSet);
  }
  if(channel->events & WriteEvent)
  {
    FD_SET(channel->fd, &data->writeSet);
  }
}

static void clearFdSet(struct Channel* channel, struct SelectData* data)
{
  FD_CLR(channel->fd, &data->readSet);
  FD_CLR(channel->fd, &data->writeSet);
}

static void* selectinit()
{
  struct SelectData* data = (struct SelectData*)malloc(sizeof(struct SelectData));
  if(data == NULL)
  {
    perror("malloc");
    return NULL;
  }

  FD_ZERO(&data->readSet);
  FD_ZERO(&data->writeSet);
  return data;
}

static int selectadd(struct Channel* channel, struct EventLoop* evloop)
{
  if(channel->fd >= Max)
  {
    return -1;
  }

  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  setFdSet(channel, data);
  return 0;
}

static int selectremove(struct Channel* channel, struct EventLoop* evloop)
{
  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  clearFdSet(channel, data);

  if(channel->destoryCallback != NULL)
  {
    channel->destoryCallback(channel->arg);
  }

  return 0;
}

static int selectmodify(struct Channel* channel, struct EventLoop* evloop)
{
  if(channel->fd >= Max)
  {
    return -1;
  }

  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  clearFdSet(channel, data);
  setFdSet(channel, data);
  return 0;
}

static int selectdispatch(struct EventLoop* evloop, int timeout)
{
  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  struct timeval val;
  val.tv_sec = timeout;
  val.tv_usec = 0;

  fd_set rdtmp = data->readSet;
  fd_set wrtmp = data->writeSet;

  int count = select(Max, &rdtmp, &wrtmp, NULL, &val);
  if(count == -1)
  {
    if(errno == EINTR)
    {
      return 0;
    }
    perror("select");
    return -1;
  }

  for(int i = 0; i < Max && count > 0; ++i)
  {
    if(FD_ISSET(i, &rdtmp))
    {
      eventActivate(evloop, i, ReadEvent);
      count--;
    }
    if(FD_ISSET(i, &wrtmp))
    {
      eventActivate(evloop, i, WriteEvent);
      count--;
    }
  }

  return 0;
}

static int selectclear(struct EventLoop* evloop)
{
  free(evloop->dispatcherdata);
  return 0;
}
