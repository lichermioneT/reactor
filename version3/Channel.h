#pragma once
#include <stdbool.h>

// 1.函数指针
typedef int(*handleFunc)(void* arg);

// 2.c语言的标志位,文件描述符的读写事件
enum FDEvent
{
  // TimeOut = 0x01,
  ReadEvent = 0x02,  // 二进制：10
  WriteEvent = 0x04  // 二进制：100
};

struct Channel
{
  //1.文件描述符
  int fd;
  //2.需要处理的事件
  int events;
  //3.处理时间对应的函数
  handleFunc readCallback;
  handleFunc writeCallback;
  
  //4.事件处理函数，对应的参数(不清楚，设置为泛型)
  void* arg;
};

// 1.初始化一个Channel
struct Channel* channelInit(int fd, int events, handleFunc reaFunc, handleFunc writeFunc, void* arg);

// 2.修改fd的写事件(检测，或者不被检查)
void writeEventEnable(struct Channel* channel, bool flag);

// 3.判断是否需要检查文件描述符的写事件
bool isWriteEventEnable(struct Channel* channel);

// 这三个函数就是 Channel 模块的基础操作：初始化一个 fd 事件对象，并通过位运算动态控制它是否监听写事件。
// channel->events |= WriteEvent;      // 开启写事件
// channel->events &= ~WriteEvent;     // 关闭写事件
// channel->events & WriteEvent;       // 判断写事件
