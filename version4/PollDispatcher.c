#include "Dispatcher.h"
#include "EventLoop.h"
#include "Channel.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

static void* pollinit();
static int polladd(struct Channel* channel, struct EventLoop* evloop);
static int pollremove(struct Channel* channel, struct EventLoop* evloop);
static int pollmodify(struct Channel* channel, struct EventLoop* evloop);
static int polldispatch(struct EventLoop* evloop, int timeout);
static int pollclear(struct EventLoop* evloop);

// MODIFIED: restored poll dispatcher declaration and static prototypes.
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

static void* pollinit()
{
  struct PollData* data = (struct PollData*)malloc(sizeof(struct PollData));
  if(data == NULL)
  {
    perror("malloc");
    return NULL;
  }

  data->maxfd = 0;
  for(int i = 0; i < Max; ++i)
  {
    data->fds[i].fd = -1;
    data->fds[i].events = 0;
    data->fds[i].revents = 0;
  }

  return data;
}

static short pollEvents(struct Channel* channel)
{
  short events = 0;
  if(channel->events & ReadEvent)
  {
    events |= POLLIN;
  }
  if(channel->events & WriteEvent)
  {
    events |= POLLOUT;
  }
  return events;
}

static int polladd(struct Channel* channel, struct EventLoop* evloop)
{
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  int i = 0;
  for(; i < Max; ++i)
  {
    if(data->fds[i].fd == -1)
    {
      data->fds[i].events = pollEvents(channel);
      data->fds[i].revents = 0;
      data->fds[i].fd = channel->fd;
      data->maxfd = i > data->maxfd ? i : data->maxfd;
      return 0;
    }
  }

  return -1;
}

static int pollremove(struct Channel* channel, struct EventLoop* evloop)
{
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  int found = -1;
  for(int i = 0; i < Max; ++i)
  {
    if(data->fds[i].fd == channel->fd)
    {
      data->fds[i].events = 0;
      data->fds[i].revents = 0;
      data->fds[i].fd = -1;
      found = i;
      break;
    }
  }

  while(data->maxfd > 0 && data->fds[data->maxfd].fd == -1)
  {
    data->maxfd--;
  }

  if(channel->destoryCallback != NULL)
  {
    channel->destoryCallback(channel->arg);
  }

  return found == -1 ? -1 : 0;
}

static int pollmodify(struct Channel* channel, struct EventLoop* evloop)
{
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  for(int i = 0; i < Max; ++i)
  {
    if(data->fds[i].fd == channel->fd)
    {
      data->fds[i].events = pollEvents(channel);
      return 0;
    }
  }

  return -1;
}

static int polldispatch(struct EventLoop* evloop, int timeout)
{
  struct PollData* data = (struct PollData*)evloop->dispatcherdata;
  int count = poll(data->fds, (nfds_t)(data->maxfd + 1), timeout * 1000);
  if(count == -1)
  {
    if(errno == EINTR)
    {
      return 0;
    }
    perror("poll");
    return -1;
  }

  for(int i = 0; i <= data->maxfd && count > 0; ++i)
  {
    if(data->fds[i].fd == -1 || data->fds[i].revents == 0)
    {
      continue;
    }

    if(data->fds[i].revents & POLLIN)
    {
      eventActivate(evloop, data->fds[i].fd, ReadEvent);
    }
    if(data->fds[i].revents & POLLOUT)
    {
      eventActivate(evloop, data->fds[i].fd, WriteEvent);
    }
    count--;
  }

  return 0;
}

static int pollclear(struct EventLoop* evloop)
{
  free(evloop->dispatcherdata);
  return 0;
}
