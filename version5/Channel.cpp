#include "Channel.h"

Channel::Channel(int fd, int events, void* arg, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryCallback)
  :_fd(fd)
  ,_events(events)
  ,_arg(arg)
  ,_readCallback(readFunc)
  ,_writeCallback(writeFunc)
  ,_destroyCallback(destoryCallback)
{}

void Channel::writeEventEnable(bool flag)
{
  if(flag)
  {
    _events |= static_cast<int>(FDEvent::WriteEvent);
  }
  else
  {
    _events &= ~static_cast<int>(FDEvent::WriteEvent);
  }
}

bool Channel::isWriteEventEnable() const
{
  return _events & static_cast<int>(FDEvent::WriteEvent);
}
