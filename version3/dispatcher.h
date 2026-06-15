#pragma once
#include "channel.h"
#include "evnetloop.h"

struct dispatcher
{
  // 初始化 epoll, poll, select需要的数据块
  void*(*init)();
  // 添加
  int(*add)(struct Channel* channel, struct eventloop* evloop);
  // 删除
  int(*remove)(struct Channel* channel, struct eventloop* evloop);
  // 修改
  int(*modify)(struct Channel* channel, struct eventloop* evloop);
  // 事件监测
  int(*dispatcher)(struct eventloop* evloop, int timeout);
  // 资源释放 
  int(*clear)(struct eventloop* evloop);
};
