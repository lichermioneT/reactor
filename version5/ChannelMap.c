#include "ChannelMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ChannelMap* channelMapInit(int size)
{
  if(size <= 0)
  {
    return NULL;
  }

  struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
  if(map == NULL)
  {
    perror("malloc");
    return NULL;
  }

  map->list = (struct Channel**)calloc((size_t)size, sizeof(struct Channel*));
  if(map->list == NULL)
  {
    perror("calloc");
    free(map);
    return NULL;
  }

  map->size = size;
  return map;
}

void channelMapClear(struct ChannelMap* map)
{
  if(map == NULL)
  {
    return;
  }

  free(map->list);
  map->list = NULL;
  map->size = 0;
  free(map);
}

// MODIFIED: keep newSize as required capacity and guard zero-sized maps.
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize)
{
  if(map == NULL || unitSize <= 0 || newSize <= 0)
  {
    return false;
  }

  if(map->size >= newSize)
  {
    return true;
  }

  int curSize = map->size > 0 ? map->size : 1;
  while(curSize < newSize)
  {
    curSize *= 2;
  }

  struct Channel** temp = (struct Channel**)realloc(map->list,
                                                     (size_t)curSize * (size_t)unitSize);
  if(temp == NULL)
  {
    perror("realloc");
    return false;
  }

  map->list = temp;
  memset(&map->list[map->size], 0, (size_t)(curSize - map->size) * (size_t)unitSize);
  map->size = curSize;
  return true;
}
