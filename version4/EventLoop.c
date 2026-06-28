#include "EventLoop.h"

#include "Channel.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void taskWakeup(struct EventLoop* evLoop)
{
  const char* msg = "wakeup";
  if(evLoop != NULL)
  {
    write(evLoop->socketPair[0], msg, strlen(msg));
  }
}

static int readLocalMessage(void* arg)
{
  struct EventLoop* evLoop = (struct EventLoop*)arg;
  char buf[256] = {0};
  if(evLoop != NULL)
  {
    read(evLoop->socketPair[1], buf, sizeof(buf));
  }
  return 0;
}

struct EventLoop* EventLoopInit()
{
  return EventLoopInitEx(NULL);
}

struct EventLoop* EventLoopInitEx(const char* threadName)
{
  struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
  if(evLoop == NULL)
  {
    perror("malloc");
    return NULL;
  }

  evLoop->isQuit = false;
  evLoop->dispatcher = &EpollDispatcher;
  evLoop->dispatcherdata = NULL;
  evLoop->head = NULL;
  evLoop->tail = NULL;
  evLoop->channelMap = NULL;
  evLoop->threadID = pthread_self();
  evLoop->socketPair[0] = -1;
  evLoop->socketPair[1] = -1;
  pthread_mutex_init(&evLoop->mutex, NULL);
  snprintf(evLoop->threadName, sizeof(evLoop->threadName), "%s",
           threadName == NULL ? "main thread" : threadName);

  evLoop->dispatcherdata = evLoop->dispatcher->init();
  evLoop->channelMap = channelMapInit(128);
  if(evLoop->dispatcherdata == NULL || evLoop->channelMap == NULL)
  {
    free(evLoop);
    return NULL;
  }

  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketPair);
  if(ret == -1)
  {
    perror("socketpair");
    free(evLoop);
    return NULL;
  }

  struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent,
                                        readLocalMessage, NULL, NULL, evLoop);
  if(channel == NULL)
  {
    close(evLoop->socketPair[0]);
    close(evLoop->socketPair[1]);
    free(evLoop);
    return NULL;
  }

  eventLoopAddTask(evLoop, channel, ADD);
  return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop)
{
  assert(evLoop != NULL);

  if(evLoop->threadID != pthread_self())
  {
    return -1;
  }

  struct Dispatcher* dispatcher = evLoop->dispatcher;
  while(!evLoop->isQuit)
  {
    dispatcher->dispatch(evLoop, 2);
    eventLoopProcessTask(evLoop);
  }

  return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
  if(fd < 0 || evLoop == NULL || evLoop->channelMap == NULL ||
     fd >= evLoop->channelMap->size)
  {
    return -1;
  }

  struct Channel* channel = evLoop->channelMap->list[fd];
  if(channel == NULL || channel->fd != fd)
  {
    return -1;
  }

  if((event & ReadEvent) && channel->readCallback != NULL)
  {
    channel->readCallback(channel->arg);
  }
  if((event & WriteEvent) && channel->writeCallback != NULL)
  {
    channel->writeCallback(channel->arg);
  }

  return 0;
}

int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
  if(evLoop == NULL || channel == NULL)
  {
    return -1;
  }

  struct ChannelElement* node =
      (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
  if(node == NULL)
  {
    perror("malloc");
    return -1;
  }

  node->channel = channel;
  node->type = type;
  node->next = NULL;

  pthread_mutex_lock(&evLoop->mutex);
  if(evLoop->head == NULL)
  {
    evLoop->head = node;
    evLoop->tail = node;
  }
  else
  {
    evLoop->tail->next = node;
    evLoop->tail = node;
  }
  pthread_mutex_unlock(&evLoop->mutex);

  if(evLoop->threadID == pthread_self())
  {
    eventLoopProcessTask(evLoop);
  }
  else
  {
    taskWakeup(evLoop);
  }

  return 0;
}

int eventLoopProcessTask(struct EventLoop* evLoop)
{
  if(evLoop == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&evLoop->mutex);
  struct ChannelElement* head = evLoop->head;
  evLoop->head = NULL;
  evLoop->tail = NULL;
  pthread_mutex_unlock(&evLoop->mutex);

  while(head != NULL)
  {
    struct ChannelElement* tmp = head;
    struct Channel* channel = head->channel;

    if(head->type == ADD)
    {
      eventLoopAdd(evLoop, channel);
    }
    else if(head->type == DELETE)
    {
      eventLoopRemove(evLoop, channel);
    }
    else if(head->type == MODIFY)
    {
      eventLoopModify(evLoop, channel);
    }

    head = head->next;
    free(tmp);
  }

  return 0;
}

int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel)
{
  if(evLoop == NULL || channel == NULL || evLoop->channelMap == NULL)
  {
    return -1;
  }

  int fd = channel->fd;
  if(fd < 0)
  {
    return -1;
  }

  struct ChannelMap* channelMap = evLoop->channelMap;
  if(fd >= channelMap->size)
  {
    // MODIFIED: fd is an index, so required capacity is fd + 1.
    if(!makeMapRoom(channelMap, fd + 1, sizeof(struct Channel*)))
    {
      return -1;
    }
  }

  if(channelMap->list[fd] == NULL)
  {
    channelMap->list[fd] = channel;
    return evLoop->dispatcher->add(channel, evLoop);
  }

  return 0;
}

int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel)
{
  if(evLoop == NULL || channel == NULL || evLoop->channelMap == NULL)
  {
    return -1;
  }

  int fd = channel->fd;
  if(fd < 0 || fd >= evLoop->channelMap->size)
  {
    return -1;
  }

  return evLoop->dispatcher->remove(channel, evLoop);
}

int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel)
{
  if(evLoop == NULL || channel == NULL || evLoop->channelMap == NULL)
  {
    return -1;
  }

  int fd = channel->fd;
  if(fd < 0 || fd >= evLoop->channelMap->size ||
     evLoop->channelMap->list[fd] == NULL)
  {
    return -1;
  }

  return evLoop->dispatcher->modify(channel, evLoop);
}

int destroyChannel(struct EventLoop* evLoop, struct Channel* channel)
{
  if(evLoop == NULL || channel == NULL)
  {
    return -1;
  }

  if(evLoop->channelMap != NULL && channel->fd >= 0 &&
     channel->fd < evLoop->channelMap->size)
  {
    evLoop->channelMap->list[channel->fd] = NULL;
  }

  close(channel->fd);
  free(channel);
  return 0;
}
