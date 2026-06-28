#pragma once

#include <stdbool.h>

struct Channel;

struct ChannelMap
{
  int size;
  struct Channel** list;
};

// MODIFIED: restored struct closing brace and map API declarations.
struct ChannelMap* channelMapInit(int size);
void channelMapClear(struct ChannelMap* map);
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);
