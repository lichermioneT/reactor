#pragma once

struct ChannelMap
{
  int size; // 指针指向数组的元素个数
  // struct Channel* list[]
  struct Channel** list; // 指向---》指针数组
};

// 1.初始化,开辟空间的
struct ChannelMap* channelMapInit(int size);
// 2.清空map
void channelMapClear(struct ChannelMap* map);
// 3.重新进行扩容
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);
// 用一个数组保存 fd 和 Channel 的映射关系。
