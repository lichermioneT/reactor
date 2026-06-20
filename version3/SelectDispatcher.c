#include "Dispatcher.h"
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

/*
selectdispatch模块的总结
1.开辟额外的空间给select使用
2.channel合格就添加到反应堆
3.channel合格就从反映里面修改读写
4.channel合计就从反应堆里面删除
5.开始监听
6.删除额外的数组
*/

/*
selectdispatch模块的总结
1.初始化函数, 初始化select读写需要额外的数组
2.添加函数，读写事件添加到上面的数组，后续的监听
3.移除函数，读写事件从上面的数组里面进行删除了
4.修改：添加或者删除
5.事件的检查，然后分发给读写函数
6.清理刚才开辟的数组
*/


// Dispatcher 是底层事件检测机制的抽象接口；
// EventLoop 是上层事件循环对象，它通过 Dispatcher 去管理和分发 fd 事件

// 静态函数的声明
// 1.初始化数据块的。eselect,select,select。返回值兼容三种数据类型的。
static void* selectinit(); 
// 2.添加文件描述符。(文件描述符在channel里面封装了的。)
static int selectadd(struct Channel* channel, struct EventLoop* evloop); 
// 3.删除节点
static int selectremove(struct Channel* channel, struct EventLoop* evloop); 
// 4.修改的
static int selectmodify(struct Channel* channel, struct EventLoop* evloop); 
// 5.事件的检测
static int selectdispatch(struct EventLoop* evloop, int timeout); 
 //6.关闭文件描述符
static int selectclear(struct EventLoop* evloop); 

// 7.两个辅助函数
static void setFdSet(struct Channel* channel, struct SelectData* data);
static void clearFdSet(struct Channel* channel, struct SelectData* data);

// Dispatcher分发起
struct Dispatcher SelectDispatcher =
{
  selectinit,
  selectadd,
  selectremove,
  selectmodify,
  selectdispatch,
  selectclear,
};

#define Max 1024
struct SelectData
{
  fd_set readSet;
  fd_set writeSet;
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
  if(channel->events & ReadEvent)
  {
    FD_CLR(channel->fd, &data->readSet);
  }
  if(channel->events & WriteEvent)
  {
    FD_CLR(channel->fd, &data->writeSet);
  }
}

// 初始化eselect需要的数据
static void* selectinit()
{
  // 1.开辟堆空间，存放select需要的数据
  struct SelectData* data = (struct SelectData*)malloc(sizeof(struct SelectData));

  // 2.清空集合
  FD_ZERO(&data->readSet); 
  FD_ZERO(&data->writeSet);
 
  // 3.数据块返还给调用者
  return data;
}

// 1.添加
static int selectadd(struct Channel* channel, struct EventLoop* evloop)
{
  // 1.EventLoop里面的dispatcherdata里面拿数据的。
  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  
  // 2.select只能给检查1024个文件描述符的
  if(channel->fd >= Max)
  {
    return -1;
  }

  setFdSet(channel, data);

  return 0;
}

// 2.删除
static int selectremove(struct Channel* channel, struct EventLoop* evloop)
{
  // 1.EventLoop里面的dispatcherdata里面拿数据的。
  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  
  // 2.清除事件的读写事件
  clearFdSet(channel, data);

  return 0;
}

// 3.修改
static int selectmodify(struct Channel* channel, struct EventLoop* evloop)
{
  // 1.EventLoop里面的dispatcherdata里面拿数据的。
  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  setFdSet(channel, data);
  clearFdSet(channel, data);

  return 0;
}

// 4.事件的检测
static int selectdispatch(struct EventLoop* evloop, int timeout)
{
  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  struct timeval val;
  val.tv_sec =  timeout;
  val.tv_usec =  0;
  
  fd_set rdtmp = data->readSet;
  fd_set wrtmp = data->writeSet;

  int count = select(Max, &rdtmp, &wrtmp, NULL, &val);
  if(count == -1)
  {
    perror("select");
    exit(0);
  }

  // 激活读写事件
  for(int i = 0; i < Max; ++i)
  {
    if(FD_ISSET(i, &rdtmp))
    {
      eventActivate(evloop, i, ReadEvent);
    }
    if(FD_ISSET(i, &wrtmp))
    {
      eventActivate(evloop, i, WriteEvent);
    }
  }

  return 0;
}
 //6.关闭文件描述符
static int selectclear(struct EventLoop* evloop)
{
  struct SelectData* data = (struct SelectData*)evloop->dispatcherdata;
  free(data);
  return 0;
}
