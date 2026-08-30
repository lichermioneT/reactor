#pragma once 
#include <stdbool.h>

// channel总结：
// 文件描述符的封装，然后处理读写的事件

// 定义函数指针
typedef int(*handleFunc)(void* arg);

// 定义文件描述符的读写事件
enum FDEvent
{
  ReadEvent = 0x02,  // 0010
  WriteEvent = 0x04  // 0100
};

struct Channel 
{
  int fd; // 文件描述
  int events; // 事件
  handleFunc readCallback;  // 读回调
  handleFunc writeCallback; // 写回调
  void* arg; // 回调函数的参数
};

// 初始化一个Channel;
struct Channel* channelInit(int fd, int events, handleFunc readFunc, handleFunc writeFunc, void* arg);
// 修改fd的写事件(监测或者不检查)
void writeEventEnable(struct Channel* channel, bool flag);
// 判断是否需要检查文件描述符的写事件
bool isWriteEventEnable(struct Channel* channel);
