#include "ChannelMap.h"

ChannelMap::ChannelMap(int size)
  :_size(size)
  ,_list(size)
{
  for(int i = 0; i < size; ++i)
  {
    _list[i] = nullptr;
  }
}

ChannelMap::~ChannelMap()
{
  for(int i = 0; i < _size; ++i)
  {
    if(*_list[i])
    {
      free(*_list[i]);
    }
  }
}

