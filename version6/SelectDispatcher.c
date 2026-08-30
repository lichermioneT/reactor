#include "Dispatcher.h"
#include <unistd.h>
#include "Channel.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include "EventLoop.h"

#define Max 1024

struct SelectData 
{
  fd_set readSet;
  fd_set writeSet;
};

  // 初始化
static  void* selectinit();
  // 添加
static int selectAdd(struct Channel* channel, struct EventLoop* evLoop);
  // 删除
static int selectRemove(struct Channel* channel, struct EventLoop* evLoop);
  // 修改
static int selectModify(struct Channel* channel, struct EventLoop* evLoop);
  // 事件监测
static int selectDispatcher(struct EventLoop* evLoop, int timeout);
  // 清除数据
static int selectClear(struct EventLoop* evLoop);
static int setFdSet(struct Channel* channel, struct SelectData* data);
static int clearFdSet(struct Channel* channel, struct SelectData* data);

struct Dispatcher SelectDispatcher = 
{
  selectinit,
  selectAdd,
  selectRemove,
  selectModify,
  selectDispatcher,
  selectClear
};

// 初始化
static void* selectinit()
{
  struct SelectData* data = (struct SelectData*)malloc(sizeof(struct SelectData)); 
  FD_ZERO(&data->readSet);
  FD_ZERO(&data->writeSet);

  return data;
}

static int setFdSet(struct Channel* channel, struct SelectData* data)
{
  if(channel->events & ReadEvent)
  {
    FD_SET(channel->fd, &data->readSet);
  }
  if(channel->events & WriteEvent)
  {
    FD_SET(channel->fd, &data->writeSet);
  }

  return 0;
}
static int clearFdSet(struct Channel* channel, struct SelectData* data)
{
  if(channel->events & ReadEvent)
  {
    FD_CLR(channel->fd, &data->readSet);
  }
  if(channel->events & WriteEvent)
  {
    FD_CLR(channel->fd, &data->writeSet);
  }

  return 0;
}

  // 添加
static int selectAdd(struct Channel* channel, struct EventLoop* evLoop)
{
  struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;

  if(channel->fd >= Max)
  {
    return -1;
  }

  setFdSet(channel, data);

  return 0;
}
  // 删除
static int SelectRemove(struct Channel* channel, struct EventLoop* evLoop)
{
  struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
  
  clearFdSet(channel, data);

  return 0;
}
  // 修改
static int SelectModify(struct Channel* channel, struct EventLoop* evLoop)
{
  struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
  clearFdSet(channel, data);
  setFdSet(channel,data);

  return 0;
}
  // 事件监测
static int pollDispatcher(struct EventLoop* evLoop, int timeout)
{
   
  return 0;
}
  // 清除数据
static int epollClear(struct EventLoop* evLoop)
{
  struct PollData* data = (struct PollData*)evLoop->dispatcherData;
  free(data);
  return 0;
}
