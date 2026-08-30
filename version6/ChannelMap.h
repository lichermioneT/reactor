#pragma once
#include "Channel.h"
#include <stdbool.h>

// ChannelMap总结：指针数组。指向channel模块 

struct ChannelMap
{
// 数组元素个数
  int size;
// 二级指针：指针数组
  struct Channel** list;
};

// 初始化
struct ChannelMap* channelMapInit(int size);
// 清空map
void ChannelMapClear(struct ChannelMap* map);
// 扩容
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);
