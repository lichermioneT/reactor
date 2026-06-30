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

// 主线程的初始化
struct EventLoop* EventLoopInit()
{
  return EventLoopInitEx(NULL);
}

struct EventLoop* EventLoopInitEx(const char* threadName)
{
// 1.开空间
  struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
  if(evLoop == NULL)
  {
    perror("malloc");
    return NULL;
  }

// 2.初始化数据
  evLoop->isQuit = false;
  // 2.1这里选择的是epoll模型
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

  // 2.2 epoll的句柄和辅助数组
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

  // 2.3唤醒的文件描述符，注册的是读事件，readLocalMessage事件的。
  struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent,
                                        readLocalMessage, NULL, NULL, evLoop);
  if(channel == NULL)
  {
    close(evLoop->socketPair[0]);
    close(evLoop->socketPair[1]);
    free(evLoop);
    return NULL;
  }

// 3.需要唤醒的文件描述符添加到 任务队列里面去的
  eventLoopAddTask(evLoop, channel, ADD);

// 4.返还给调用者
  return evLoop;
}

// 反应堆模型启动，这里需要告诉我那个模型启动的。
int eventLoopRun(struct EventLoop* evLoop)
{
  assert(evLoop != NULL);

  if(evLoop->threadID != pthread_self())
  {
    return -1;
  }

// 1.反应堆里面拿出IO复用的模型
  struct Dispatcher* dispatcher = evLoop->dispatcher;
  while(!evLoop->isQuit)
  {
    // 1.1启动监听了，监听成功了就激活注册的事件。激活函数就在下面呢的。
    dispatcher->dispatch(evLoop, 2);

    // 1.2遍历任务队列，然后增删查改。
    eventLoopProcessTask(evLoop);
  }

  return 0;
}

// 激活读写事件
int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
  if(fd < 0 || evLoop == NULL || evLoop->channelMap == NULL ||
     fd >= evLoop->channelMap->size)
  {
    return -1;
  }

// 定位fd的位置信息
  struct Channel* channel = evLoop->channelMap->list[fd];
  if(channel == NULL || channel->fd != fd)
  {
    return -1;
  }

// channel的回调函数
  if((event & ReadEvent) && channel->readCallback != NULL)
  {
    channel->readCallback(channel->arg); // 监听文件描述符的读事件
  }
  if((event & WriteEvent) && channel->writeCallback != NULL)
  {
    channel->writeCallback(channel->arg); // 通信文件描述符的写事件。
  }

  return 0;
}

// 添加到任务队列里面去
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
  if(evLoop == NULL || channel == NULL)
  {
    return -1;
  }

  struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
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

// 遍历任务队列
// ADD,DELETE,MODIFY
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
// 1.指针数组的指针变成NULL
    evLoop->channelMap->list[channel->fd] = NULL;
  }

// 2.关闭文件描述符
  close(channel->fd);
// 3.释放空间
  free(channel);
  return 0;
}