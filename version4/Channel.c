#include "Channel.h"

#include <stdio.h>
#include <stdlib.h>

struct Channel* channelInit(int fd, int events, handleFunc reaFunc,
                            handleFunc writeFunc, handleFunc destoryCallback,
                            void* arg)
{
  struct Channel* data = (struct Channel*)malloc(sizeof(struct Channel));
  if(data == NULL)
  {
    perror("malloc");
    return NULL;
  }

  data->fd = fd;
  data->events = events;
  data->readCallback = reaFunc;
  data->writeCallback = writeFunc;
  data->destoryCallback = destoryCallback;
  data->arg = arg;

  return data;
}

void writeEventEnable(struct Channel* channel, bool flag)
{
  if(channel == NULL)
  {
    return;
  }

  if(flag)
  {
    channel->events |= WriteEvent;
  }
  else
  {
    channel->events &= ~WriteEvent;
  }
}

bool isWriteEventEnable(struct Channel* channel)
{
  return channel != NULL && (channel->events & WriteEvent);
}
