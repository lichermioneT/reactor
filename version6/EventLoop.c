#include "EventLoop.h"
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

struct EventLoop* eventLoopInit()
{
  return eventLoopInitEx(NULL);
}

void taskWakeuo(struct EventLoop* evLoop)
{
  const char* msg = "我是要成为lichermioneX的男人";
  write(evLoop->socketPair[0], msg, strlen(msg));
}

int readLocalMessage(void* arg)
{
  struct EventLoop* evLoop = (struct EventLoop*)arg;
  char buf[256] = {0};
  read(evLoop->socketPair[1], buf, sizeof(buf));
  
  return 0;
}

struct EventLoop* eventLoopInitEx(const char* threadName)
{
// 1.开空间
  struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
// 2.模型开关 
  evLoop->isQuit = false;
// 3.分发模型
  evLoop->dispatcher = &EpollDispatcher;
  evLoop->dispatcherData = evLoop->dispatcher->init();
// 4.任务队列
  evLoop->head = evLoop->tail = NULL;
// 5.map
  evLoop->map = channelMapInit(128);
// 6.线程的信息
  evLoop->threadID = pthread_self();
  pthread_mutex_init(&evLoop->mutex, NULL);
  strcpy(evLoop->threadName, threadName == NULL ? "main thread" : threadName);

// 7.线程间通信
  int ret = socketpair(AF_UNIX, SOCK_STREAM,0, evLoop->socketPair);
  if(ret == -1)
  {
    perror("socketpair");
    exit(0);
  }
  // socketpair[0]发数据
  // socketpair[1]收数据
  
  struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent, readLocalMessage, NULL, evLoop);
  // channel添加到任务队列
  eventLoopAddTask(evLoop, channel, ADD);

  return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop)
{
  assert(evLoop);
  // 取出事件分发和检查模型
  struct Dispatcher* dispatcher = evLoop->dispatcher;
    
  // 线程id是否正常
  if(evLoop->threadID != pthread_self())
  {
    return -1;
  }

  // 循环进行处理
  while(!evLoop->isQuit)
  {
    dispatcher->dispatch(evLoop, 2); // select poll epoll 
    eventLoopProcessTask(evLoop);    // 遍历任务队列
  }

  return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
  if(fd < 0 || evLoop == NULL)
  {
    return -1;
  }

  // 取出channel
  struct Channel* channel = evLoop->map->list[fd];
  assert(channel->fd == fd);
  
  if(event & ReadEvent && channel->readCallback)
  {
    channel->readCallback(channel->arg);
  }

  if(event & WriteEvent && channel->readCallback)
  {
    channel->writeCallback(channel->arg);
  }

  return 0;
}

// 添加到任务队列里面去的
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
// 加锁
  pthread_mutex_lock(&evLoop->mutex);
  
  // 创建新的节点
  struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
  node->channel = channel;
  node->type = type;
  node->next = NULL;

  // 空链表
  if(evLoop->tail == NULL)
  {
    evLoop->tail = evLoop->head = node;
  }
  else 
  {
    evLoop->tail->next = node;
    evLoop->tail = node;
  }

  pthread_mutex_unlock(&evLoop->mutex);

  // 1.修改可能是主线程或者子进程
  // 2.添加只能主线程
  // 1.主线程建立链接，子线程处理业务。
  
  if(evLoop->threadID == pthread_self())
  {
    // 当前子线程
    eventLoopProcessTask(evLoop);  
  }
  else 
  {
    // 主线程 --- 告诉子线程处理任务队列中的任务 1.子线程阻塞 2.子线程工作
    taskWakeuo(evLoop);
  }

  return 0;
}

// 遍历任务队列
int eventLoopProcessTask(struct EventLoop* evLoop)
{
  pthread_mutex_lock(&evLoop->mutex);
  
  // 头
  struct ChannelElement* head = evLoop->head;
  while(head != NULL)
  {
    struct Channel* channel = head->channel;

    if(head->type == ADD)
    {
       // 添加    
       eventLoopADD(evLoop, channel);
    }
    else if(head->type == DELETE)
    {
       //  删除
       eventLoopRemove(evLoop, channel);
    }
    else if(head->type == MOD)
    {
       // 修改
       eventLoopModify(evLoop, channel);
    }
    
    struct ChannelElement* tmp = head;
    head = head->next;
    free(tmp);
  }

  evLoop->head = evLoop->tail = NULL;
  pthread_mutex_unlock(&evLoop->mutex);

  return 0;
}

int eventLoopADD(struct EventLoop* evLoop, struct Channel* channel)
{
  int fd = channel->fd; 
  struct ChannelMap* channelMap = evLoop->map;
  
  if(fd >= channelMap->size)
  {
    // 没有足够的空间存储键值对
    if(!makeMapRoom(channelMap, fd, sizeof(struct Channel*)))
    {
      return -1;
    }
  }

  // 找到fd对应的位置
  if(channelMap->list[fd] == NULL)
  {
    channelMap->list[fd] = channel;
    evLoop->dispatcher->add(channel, evLoop);
  }

  return 0;
}

int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel)
{
  int fd = channel->fd; 
  struct ChannelMap* channelMap = evLoop->map;
  
  if(fd >= channelMap->size)
  {
    // 没有足够的空间存储键值对
      return -1;
  }

  int ret = evLoop->dispatcher->remove(channel, evLoop);

  return ret;
}

int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel)
{
  int fd = channel->fd; 
  struct ChannelMap* channelMap = evLoop->map;
  
  if(channelMap->list[fd] == NULL)
  {
    // 没有足够的空间存储键值对
      return -1;
  }

  int ret = evLoop->dispatcher->modify(channel, evLoop);
  return ret;
}

int destoryChannel(struct EventLoop* evLoop, struct Channel* channel)
{
  evLoop->map->list[channel->fd] = NULL;
  close(channel->fd);
  free(channel);

  return 0;
}
