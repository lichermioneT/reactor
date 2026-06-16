#pragma once
#include "Dispatcher.h"

// epll的分发
extern struct Dispatcher EpollDispatcher;

struct EventLoop
{
  struct Dispatcher* dispatcher; // 增删查改的方法
  void* dispatcherdata;          // 增删查改的方法的数据
};

// Dispatcher 是底层事件检测机制的抽象接口；
// EventLoop 是上层事件循环对象，它通过 Dispatcher 去管理和分发 fd 事件
