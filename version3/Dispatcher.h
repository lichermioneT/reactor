#pragma once 

struct Channel;
struct EventLoop;
/*
Dispatcher模块的总结
这里主要是六个函数指针的封装
struct Dispatcher 
{
  init 
  add 
  remove 
  modify 
  dispatch 
  clear
}

 */

// 里面存放的是函数指针，就是方法
struct Dispatcher
{
// 1.初始化数据块的。epoll,poll,select。返回值兼容三种数据类型的。
 void* (*init)(); 
// 2.添加文件描述符。(文件描述符在channel里面封装了的。)
 int(*add)(struct Channel* channel, struct EventLoop* evloop); 
// 3.删除节点
 int(*remove)(struct Channel* channel, struct EventLoop* evloop); 
// 4.修改的
 int(*modify)(struct Channel* channel, struct EventLoop* evloop); 
// 5.事件的检测
 int(*dispatch)(struct EventLoop* evloop, int timeout); 
 //6.关闭文件描述符
 int(*clear)(struct EventLoop* evloop); 
};

// Dispatcher 是底层事件检测机制的抽象接口；
// EventLoop 是上层事件循环对象，它通过 Dispatcher 去管理和分发 fd 事件
