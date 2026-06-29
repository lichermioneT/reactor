#include "Dispatcher.h"
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include "SelectDispatcher.h"

SelectDispatcher::SelectDispatcher(EventLoop* evloop) :Dispatcher(evloop)
{
    FD_ZERO(&_readSet);
    FD_ZERO(&_writeSet);
    _name = "Select";
}

SelectDispatcher::~SelectDispatcher()
{
}

int SelectDispatcher::add()
{
    if (_channel->getSocket() >= _maxSize)
    {
        return -1;
    }
    setFdSet();
    return 0;
}

int SelectDispatcher::remove()
{
    clearFdSet();
    // 通过 channel 释放对应的 TcpConnection 资源
    if (_channel->_destroyCallback)
    {
        _channel->_destroyCallback(const_cast<void*>(_channel->getArg()));
    }

    return 0;
}

int SelectDispatcher::modify()
{
    clearFdSet();
    setFdSet();
    return 0;
}

int SelectDispatcher::dispatch(int timeout)
{
    struct timeval val;
    val.tv_sec = timeout;
    val.tv_usec = 0;
    fd_set rdtmp = _readSet;
    fd_set wrtmp = _writeSet;
    int count = select(_maxSize, &rdtmp, &wrtmp, NULL, &val);
    if (count == -1)
    {
        perror("select");
        exit(0);
    }
    if (count == 0)
    {
        return 0;
    }
    for (int i = 0; i < _maxSize; ++i)
    {
        if (FD_ISSET(i, &rdtmp))
        {
            _evLoop->eventActive(i, (int)FDEvent::ReadEvent);
        }

        if (FD_ISSET(i, &wrtmp))
        {
            _evLoop->eventActive(i, (int)FDEvent::WriteEvent);
        }
    }
    return 0;
}

void SelectDispatcher::setFdSet()
{
    if (_channel->getEvent() & (int)FDEvent::ReadEvent)
    {
        FD_SET(_channel->getSocket(), &_readSet);
    }
    if (_channel->getEvent() & (int)FDEvent::WriteEvent)
    {
        FD_SET(_channel->getSocket(), &_writeSet);
    }
}

void SelectDispatcher::clearFdSet()
{
    FD_CLR(_channel->getSocket(), &_readSet);
    FD_CLR(_channel->getSocket(), &_writeSet);
}
