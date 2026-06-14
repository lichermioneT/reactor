#include <stdlib.h>
#include <stdbool.h>
#include "channelmap.h"

struct ChannelMap* channelMapInit(int size)
{
  struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
  map->size = size;
  map->list = (struct Channel**)malloc(size * sizeof(struct Channel*));
  
  return map;
}

void ChannelMapClear(struct ChannelMap* map)
{
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
  }
  map->size = 0;
}


bool makeMapRoom(struct ChannelMap* map, int newSize, int unitsize)
{
    

}
