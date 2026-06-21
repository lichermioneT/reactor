#include "EventLoop.h"
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/*
1.初始化函数，除了主线程的名字不一样，都是一样的，申请一个空间 strcut EventLoop存放数据信息。
2.run函数，取出反应堆的dispatcher模块，开始进行事件的监听任务。然后执行对应的读写回调。
*/

// 写数据,写的时候，读数据就被唤醒了的。
void taskWakeup(struct EventLoop* evLoop)
{
  const char* msg = "我是要成为lichermionex的男人";
  write(evLoop->socketPair[0], msg, strlen(msg));
}

// 读数据。写了数据，读数据的时候就被激活了的。
int readLocalMessage(void* arg)
{
  struct EventLoop* evLoop = (struct EventLoop*)arg;
  char buf[256] = {0};
  read(evLoop->socketPair[1], buf, sizeof(buf));
  return 0;
}

// 主线程唤醒子线程----通过一个socketpair就被激活了的。处理任务队列的任务。
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
  
  // 线程id,锁的初始化。
  evLoop->isQuit = false;
  evLoop->threadID = pthread_self();
  pthread_mutex_init(&evLoop->mutex, NULL);
  strcpy(evLoop->threadName, threadName == NULL ? "main thread" : threadName); // 线程的名称进行复制2

// 2.这里指定的是epoll模型的,实例的指定。
  evLoop->dispatcher = &EpollDispatcher;              // 指定的是epoll模型的
  evLoop->dispatcherdata = evLoop->dispatcher->init();// epoll需要的额外辅助空间。
  
// 3.链表初始化为空的
  evLoop->head = evLoop->tail = NULL;

// 4.map开辟128个空间存放channel。已经实现好了的。
// 128个数组，用来存放channel
  evLoop->channelMap = channelMapInit(128);
 
// 5.本地通信的的文件描述符,0发数据， 1写数据
//   用来唤醒对应的线程信息。
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

// 启动的示例是谁，就启动谁。
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
    dispatcher->dispatch(evLoop, 2); // 监听到任务了，就执行对应的读写回调函数。这里是epoll_wait执行。
    eventLoopProcessTask(evLoop);    // 处理任务队列的任务
  }

  return 0;
}

// 处理激活文件描述符fd
int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
  // 判断是否是有效的数据
  if(fd < 0 || evLoop == NULL)
  {
    return -1;
  }

// 已经激活的文件描述符fd，是否在channelmap里面的。
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

/*
 *先入队，再判断线程：
 *
 *自己线程提交 → 自己立即处理；
 *其他线程提交 → 只唤醒目标线程，让目标线程处理。
 */
// 这里是添加到任务队列里面去的。
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
// 1.加锁保护共享资源

pthread_mutex_lock(&evLoop->mutex);
  // 1.创建新的节点,然后放到任务队列里面去的。
  struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
  
  node->channel = channel;
  node->type = type;
  node->next = NULL;
  
  if(evLoop->head == NULL) // 空链表
  {
    evLoop->head = evLoop->tail = node;
  }
  else // 非空链表
  {
    evLoop->tail->next = node; // 添加
    evLoop->tail = node;       // 后移
  }

pthread_mutex_unlock(&evLoop->mutex);

  // 2.处理节点
  // 对应链表节点：可能是主线程也可能是其它线程进行添加的
  //   1).修改fd的事件，子线程发起，子线程处理
  //   2).添加新的fd,   主线程发起，主线程处理
  // 不能让主线程处理任务队列，需要由当前的子线程去处理
  // 

  if(evLoop->threadID == pthread_self())
  {
     // 当前子线程,处理任务队列的任务。遍历任务队列。
    eventLoopProcessTask(evLoop);   
  }
  else 
  {
    // 主线程:告诉子线程处理任务队列中的任务。
    // 1.子线程在工作
    // 2.子线程被阻塞了
    // 唤醒子线程在epoll, poll, select模块的阻塞，然后来处理任务队列的，
    
    // 我们放一个文件描述符，然后控制这个文件描述符的。唤醒子进程
    taskWakeup(evLoop);
  }

  return 0;
}

// 处理任务队列里面的任务。
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

//把任务队列的channel添加到channelmap里面去的。 
//dispatcher检查集合里面去的。
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

// dispatcherdata里面进行删除
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

// dispatcherdata里面进行修改读写事件
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

// 删除channel，就是删除文件描述符。
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
