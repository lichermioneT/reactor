#include "Channel.h"

Channel::Channel(int fd, FDEvent evets, handleFunc readCallback, handleFunc writeCallback, void* arg) 
  :_fd(fd)
  ,_evets(evets)
  ,_readCallback(readCallback)
  ,_writeCallback(writeCallback)
  ,_arg(arg)
  {}

void Channel::writeEventEnable(bool flag)
{
  unsigned char evets = static_cast<unsigned char>(_evets);
  unsigned char writeEvent = static_cast<unsigned char>(FDEvent::WriteEvent);

  if(flag)
  {
    evets |= writeEvent;
  }
  else 
  {
    evets &= ~writeEvent;
  }

  _evets = static_cast<FDEvent>(evets);
}

bool Channel::isWriteEnanle()
{
    unsigned char events = static_cast<unsigned char>(_evets);

    unsigned char writeEvent = static_cast<unsigned char>(FDEvent::WriteEvent);

    return (events & writeEvent) != 0;
}


