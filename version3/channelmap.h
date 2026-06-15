#pragma once
#include "channel.h"

// 这里是存放size个Channel指针的。
struct ChannelMap
{
  int size; // 记录指针指向指针数组元素个数
  struct Channel** list;
};

// 初始化
struct ChannelMap* channelMapInit(int size);
// 清空
void ChannelMapClear(struct ChannelMap* map);
// 重新分配空间
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitsize);
