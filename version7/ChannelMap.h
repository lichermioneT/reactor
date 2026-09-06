#pragma  once 
#include "Channel.h"
#include <vector>

using std::vector;

class ChannelMap
{
public:
  ChannelMap(int size);
  ~ChannelMap();

private:
  int _size;
  vector<Channel**> _list;
};

