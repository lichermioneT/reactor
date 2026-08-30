#pragma once

#include <stdbool.h>

struct Channel;

struct ChannelMap
{
  int size;               // 数组的大小
  struct Channel** list;  // 指针数组
};

// MODIFIED: restored struct closing brace and map API declarations.
struct ChannelMap* channelMapInit(int size);
void channelMapClear(struct ChannelMap* map);
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);

/*
提供一个指针数组，维护文件描述符和channel的映射关系。
*/