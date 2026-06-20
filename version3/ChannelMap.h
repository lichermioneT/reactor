#pragma once
/*
ChannelMap模块总结
1.主要是一个指针数据，用来存放channel的
2.Channel* c1 Channel* c2 ... Channel* cn 
3.size == cn。list里面的元素存放的就是c1,,,,cn的地址(这就是二级指针了的)
4.1开辟size个大小的指针数组
4.2情况空间
4.3扩容函数的。
*/


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
