#include "EventLoop.h"
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// 写数据
void taskWakeup(struct EventLoop* evLoop)
{
  const char* msg = "我是要成为lichermionex的男人";
  write(evLoop->socketPair[0], msg, strlen(msg));
}

// 读数据
int readLocalMessage(void* arg)
{
  struct EventLoop* evLoop = (struct EventLoop*)arg;
  char buf[256] = {0};
  read(evLoop->socketPair[1], buf, sizeof(buf));
  return 0;
}

// 1.主线程初始化EventLoop模块
struct EventLoop* EventLoopInit()
{
  return EventLoopInitEx(NULL);
}

// 2.子线程初始化EventLoop模块
struct EventLoop* EventLoopInitEx(const char* threadName)
{
  // 1.开空间存放数据
  struct EventLoop*  evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
  if(evLoop == NULL)
  {
    perror("malloc");
    return NULL;
  }

  evLoop->isQuit = false;
  evLoop->threadID = pthread_self();
  pthread_mutex_init(&evLoop->mutex, NULL);
  strcpy(evLoop->threadName, threadName == NULL ? "main thread" : threadName);

  // 2.这里指定的是epoll模型的
  evLoop->dispatcher = &EpollDispatcher;              // 指定的是epoll模型的
  evLoop->dispatcherdata = evLoop->dispatcher->init();// epoll需要的赋值数据
  
  // 3.链表初始化为空的
  evLoop->head = evLoop->tail = NULL;
  // 4.map开辟128个空间存放channel
  evLoop->channelMap = channelMapInit(128);
 
// 5.本地通信的的文件描述符,0发数据， 1写数据
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketPair);
  if(ret  == -1)
  {
    perror("socketpair");
    exit(0);
  }

// 6.这里注册一个环形的fd，只是关系它的读事件
  struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent, readLocalMessage, NULL, evLoop);
  
// 7.添加到队列里面去
  eventLoopAddTask(evLoop, channel, ADD);

// 8.返回数据
  return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop)
{
  assert(evLoop != NULL);
  // dispatcher里面的epoll,poll,select.
  // 1.取出事件分发器
  struct Dispatcher* dispatcher = evLoop->dispatcher;

  // 比较线程id是否正常
  if(evLoop->threadID != pthread_self())
  {
    return -1;
  }

  // 2.循环进行事件的检查
  while(!evLoop->isQuit)
  {
    dispatcher->dispatch(evLoop, 2);
    eventLoopProcessTask(evLoop);
  }

  return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
  // 判断是否是有效的数据
  if(fd < 0 || evLoop == NULL)
  {
    return -1;
  }

  // 1.取出channel的fd和已经存在的fd。
  struct Channel* channel = evLoop->channelMap->list[fd];
  assert(channel->fd == fd);

  // 2.处理读写事件
  if(event & ReadEvent && channel->readCallback)
  {
    channel->readCallback(channel->arg);
  }
  if(event & WriteEvent && channel->writeCallback)
  {
    channel->writeCallback(channel->arg);
  }

  return 0;
}

int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
// 1.加锁保护共享资源
  pthread_mutex_lock(&evLoop->mutex);
  // 1.创建新的节点
  struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
  
  node->channel = channel;
  node->type = type;
  node->next = NULL;
  
  if(evLoop->head == NULL)
  {
    evLoop->head = evLoop->tail = node;
  }
  else 
  {
    evLoop->tail->next = node; // 添加
    evLoop->tail = node;       // 后移
  }

  pthread_mutex_unlock(&evLoop->mutex);

  // 处理节点
  // 对应链表节点：可能是主线程也可能是其它线程进行添加的
  //
  
  if(evLoop->threadID == pthread_self())
  {
     // 当前子线程
    eventLoopProcessTask(evLoop);   
  }
  else 
  {
    // 主线程
    // 1.子线程在工作
    // 2.子线程被阻塞了
    taskWakeup(evLoop);
  }

  return 0;
}

int eventLoopProcessTask(struct EventLoop* evLoop)
{
  pthread_mutex_lock(&evLoop->mutex);
  // 1.取出头结点
  struct ChannelElement* head = evLoop->head;
  while(head != NULL)
  {
    struct Channel* channel = head->channel;

    if(head->type == ADD)
    {
      // 1.添加
      eventLoopAdd(evLoop, channel);
    }
    else if(head->type == DELETE)
    {
      // 2.删除
      eventLoopRemove(evLoop, channel);
    }
    else if(head->type == MODIFY)
    {
      // 3.修改
      eventLoopModify(evLoop, channel);

    }
    
    //4.记录一下再往后移,在释放了。 
    struct ChannelElement* tmp = head;
    head = head->next;
    free(tmp);
  }

  evLoop->head = evLoop->tail = NULL;
  pthread_mutex_unlock(&evLoop->mutex);

  return 0;
}

int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel)
{
  int fd = channel->fd;  
  struct ChannelMap* channelMap = evLoop->channelMap;
  
  // 扩容
  if(fd >= channelMap->size)
  {
    // 没有足够的空间存对应关系了，fd--channel 扩容
    if(!makeMapRoom(channelMap, fd, sizeof(struct Channel*)))
    {
      perror("makeMapRoom");
      return -1;
    }
  }

  // 找到fd对应数组元素的位置，并存储
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
  struct ChannelMap* channelMap = evLoop->channelMap;
  
  if(fd >= channelMap->size)
  {
      return -1;
  }

  int ret = evLoop->dispatcher->remove(channel, evLoop);

  return ret;
}

int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel)
{
  int fd = channel->fd;  
  struct ChannelMap* channelMap = evLoop->channelMap;
  
  if(fd >= channelMap->size || channelMap->list[fd] == NULL)
  {
      return -1;
  }

  int ret = evLoop->dispatcher->modify(channel, evLoop);
  
  return ret;
}

int destroyChannel(struct EventLoop* evLoop, struct Channel* channel)
{
  // 删除channel和fd的对应关系
  evLoop->channelMap->list[channel->fd] = NULL;

  // 关闭文件描述符
  close(channel->fd);

  // 释放
  free(channel);
  return 0;
}
