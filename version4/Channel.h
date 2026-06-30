#pragma once
#include <stdbool.h>

typedef int (*handleFunc)(void* arg);

enum FDEvent
{
  ReadEvent = 0x02, // 0010
  WriteEvent = 0x04 // 0100
};

struct Channel
{
  int fd;                     // 文件描述符
  int events;                 // 关系的事件
  handleFunc readCallback;    // 读事件
  handleFunc writeCallback;   // 写事件
  handleFunc destoryCallback; // 销毁事件
  void* arg;                  // 事件参数
};

// MODIFIED: restored fd/readCallback fields and function declarations.

// 初始化 Channel，封装 fd、监听事件、回调函数和回调参数
struct Channel* channelInit(int fd, 
                            int events, 
                            handleFunc reaFunc,
                            handleFunc writeFunc, 
                            handleFunc destoryCallback,
                            void* arg);
// 开启或关闭写事件监听
void writeEventEnable(struct Channel* channel, bool flag);

// 判断当前 Channel 是否监听了写事件
bool isWriteEventEnable(struct Channel* channel);

/*
channel模块总结
1.把文件描述符和操作文件描述符的方法放在一起的
2.控制函数，开启或者关闭写事件
3.控制函数，是否开启写事件的。
细节：读，写，销毁函数：返回值int, 参数void* arg
*/