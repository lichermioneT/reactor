#include "ChannelMap.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// 初始化
struct ChannelMap* channelMapInit(int size)
{
// 1.给map开空间
  struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
  if(map == NULL)
  {
    perror("malloc");
    return NULL;
  }
 
// 2.给list开空间
  map->size = size;
  map->list = (struct Channel**)malloc(sizeof(struct Channel) * size);
  if(map->list == NULL)
  {
    perror("malloc");
    return NULL;
  }

// 返回给调用者
  return map;
}

// 清空map
void ChannelMapClear(struct ChannelMap* map)
{
  if(map)
  {
    for(int i = 0; i  < map->size; ++i)
    {
      if(map->list[i]) // 指针数据的指针指向有效的空间 在释放空间的。
      {
        free(map->list[i]); // 释放指针指向的空间
      }
    }
    free(map->list);
    map->list = NULL; // map[x1, x2,,,,,xn]; 释放数组的
  }

  map->size = 0; // 置零
}

// 扩容
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize)
{
  if(map->size > newSize)
  {
    int curSize = map->size;
    while(curSize < newSize)
    {
      curSize *= 2; // 每次扩大两倍
    }

    // 扩容
    struct Channel** temp = (struct Channel**)realloc(map->list, curSize * unitSize);
    if(temp == NULL)
    {
      return false;
    }

    map->list = temp;
    // 清理新开辟的空间
    memset(map->list[map->size], 0, (curSize - map->size) * unitSize);
    map->size = curSize;
  }

  return true;
}
