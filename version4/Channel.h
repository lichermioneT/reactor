#pragma once

#include <stdbool.h>

typedef int (*handleFunc)(void* arg);

enum FDEvent
{
  ReadEvent = 0x02,
  WriteEvent = 0x04
};

struct Channel
{
  int fd;
  int events;
  handleFunc readCallback;
  handleFunc writeCallback;
  handleFunc destoryCallback;
  void* arg;
};

// MODIFIED: restored fd/readCallback fields and function declarations.
struct Channel* channelInit(int fd, int events, handleFunc reaFunc,
                            handleFunc writeFunc, handleFunc destoryCallback,
                            void* arg);
void writeEventEnable(struct Channel* channel, bool flag);
bool isWriteEventEnable(struct Channel* channel);
