#pragma  once 
#include "Channel.h"
#include "EventLoop.h"

struct Dispatcher
{
  // 初始化--epoll poll select 所需要的数据块
  void* (*init)();

  // 添加
  int (*add)(struct Channel* channel, struct EventLoop* evLoop);
  // 删除
  int (*remove)(struct Channel* channel, struct EventLoop* evLoop);
  // 修改
  int (*modify)(struct Channel* channel, struct EventLoop* evLoop);

  // 事件监测
  int(*dispatch)(struct EventLoop* evLoop, int timeout);
  // 清除数据
  int (*clear)(struct EventLoop* evLoop);
};
