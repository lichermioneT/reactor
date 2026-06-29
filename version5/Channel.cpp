#include "Channel.h"
#include <stdlib.h>

Channel::Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg)
{
    _arg = arg;
    _fd = fd;
    _events = (int)events;
    _readCallback = readFunc;
    _writeCallback = writeFunc;
    _destroyCallback = destroyFunc;
}

void Channel::writeEventEnable(bool flag)
{
    if (flag)
    {
        // _events |= (int)FDEvent::WriteEvent;
        _events |= static_cast<int>(FDEvent::WriteEvent);
    }
    else
    {
        _events = _events & ~(int)FDEvent::WriteEvent;
    }
}

bool Channel::isWriteEventEnable()
{
    return (_events & (int)FDEvent::WriteEvent) != 0;
}
