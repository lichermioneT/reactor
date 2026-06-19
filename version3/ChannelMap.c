#include "ChannelMap.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ChannelMap是一个指针数组，里面存放的是指针。
struct ChannelMap* channelMapInit(int size)
{
  // 1.开辟一个空间struct ChannelMap。
  struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
  if(map == NULL)
  {
    perror("malloc");
    return NULL;
  }
  
  // 2.给里面的元素开辟空间，用来channel。
  map->list = (struct Channel**)malloc(size * sizeof(struct Channel*));
  memset(map->list, 0, size * sizeof(struct Channel*));
  map->size = size;

  // 3.返回map给调用者。 
  // map就是一个 文件描述符fd和channel的一个对应的。
  return map;
}

void channelMapClear(struct ChannelMap* map)
{
#if 0
  if(map != NULL)
  {
    for(int i = 0; i < map->size; ++i)
    {
      if(map->list[i] != NULL)
      {
        free(map->list[i]);
      }
    }
    free(map->list);
    map->list = NULL;
    map->size = 0;
  }
  
#else 
  if(map != NULL)
  {
    for(int i = 0; i < map->size; ++i)
    {
      if(*(map->list + i) != NULL)
      {
        free(*(map->list + i));
      }
    }
    free(map->list);
    map->list = NULL;
    map->size = 0;
  }
  
#endif
}

bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize)
{
  if(map->size < newSize)
  {
    int curSize  = map->size; 

    while(curSize < newSize)
    {
      curSize *= 2;
    }

    // 扩容
    struct Channel** temp = (struct Channel**)realloc(map->list, curSize * unitSize);
    if(temp == NULL)
    {
      return false;
    }

    map->list = temp;
    memset(&map->list[map->size], 0, (curSize - map->size) * unitSize);
    map->size = curSize;
  }

  return true;
}

bool makeMapRoom_(struct ChannelMap* map, int newSize)
{
    if (map == NULL || newSize <= 0)
    {
        return false;
    }

    if (map->size < newSize)
    {
        int oldSize = map->size;
        int curSize = map->size;

        while (curSize < newSize)
        {
            curSize *= 2;
        }

        struct Channel** temp =
            (struct Channel**)realloc(map->list, curSize * sizeof(struct Channel*));

        if (temp == NULL)
        {
            return false;
        }

        map->list = temp;

        memset(&map->list[oldSize], 0,
               (curSize - oldSize) * sizeof(struct Channel*));

        map->size = curSize;
    }

    return true;
}
