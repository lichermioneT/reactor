#include "Channel.h"

Channel::Channel(int fd, int evets, handleFunc readCallback, handleFunc writeCallback, void* arg) 
  :_fd(fd)
  ,_evets(evets)
  ,_readCallback(readCallback)
  ,_writeCallback(writeCallback)
  ,_arg(arg)
  {}

void Channel::writeEventEnable(bool flag)
{
  if(flag)
  {
    _evets |= WriteEvent;
  }
  else 
  {
    _evets &= ~WriteEvent;
  }
}

bool Channel::isWriteEnanle()
{
  return  _evets & WriteEvent;
}


