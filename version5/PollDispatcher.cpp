#include "Dispatcher.h"
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include "PollDispatcher.h"

PollDispatcher::PollDispatcher(EventLoop* evloop) : Dispatcher(evloop)
{
    _maxfd = 0;
    _fds = new struct pollfd[_maxNode];
    for (int i = 0; i < _maxNode; ++i)
    {
        _fds[i].fd = -1;
        _fds[i].events = 0;
        _fds[i].revents = 0;
    }
    _name = "Poll";
}

PollDispatcher::~PollDispatcher()
{
    delete[]_fds;
}

int PollDispatcher::add()
{
    int events = 0;
    if (_channel->getEvent() & (int)FDEvent::ReadEvent)
    {
        events |= POLLIN;
    }
    if (_channel->getEvent() & (int)FDEvent::WriteEvent)
    {
        events |= POLLOUT;
    }
    int i = 0;
    for (; i < _maxNode; ++i)
    {
        if (_fds[i].fd == -1)
        {
            _fds[i].events = events;
            _fds[i].fd = _channel->getSocket();
            _maxfd = i > _maxfd ? i : _maxfd;
            break;
        }
    }
    if (i >= _maxNode)
    {
        return -1;
    }
    return 0;
}

int PollDispatcher::remove()
{
    int i = 0;
    for (; i < _maxNode; ++i)
    {
        if (_fds[i].fd == _channel->getSocket())
        {
            _fds[i].events = 0;
            _fds[i].revents = 0;
            _fds[i].fd = -1;
            break;
        }
    }
    // 通过 channel 释放对应的 TcpConnection 资源
    if (i >= _maxNode)
    {
        return -1;
    }
    if (_channel->_destroyCallback)
    {
        _channel->_destroyCallback(const_cast<void*>(_channel->getArg()));
    }
    while (_maxfd > 0 && _fds[_maxfd].fd == -1)
    {
        --_maxfd;
    }
    return 0;
}

int PollDispatcher::modify()
{
    int events = 0;
    if (_channel->getEvent() & (int)FDEvent::ReadEvent)
    {
        events |= POLLIN;
    }
    if (_channel->getEvent() & (int)FDEvent::WriteEvent)
    {
        events |= POLLOUT;
    }
    int i = 0;
    for (; i < _maxNode; ++i)
    {
        if (_fds[i].fd == _channel->getSocket())
        {
            _fds[i].events = events;
            break;
        }
    }
    if (i >= _maxNode)
    {
        return -1;
    }
    return 0;
}

int PollDispatcher::dispatch(int timeout)
{
    int count = poll(_fds, _maxfd + 1, timeout * 1000);
    if (count == -1)
    {
        perror("poll");
        exit(0);
    }
    if (count == 0)
    {
        return 0;
    }
    for (int i = 0; i <= _maxfd; ++i)
    {
        if (_fds[i].fd == -1)
        {
            continue;
        }

        if (_fds[i].revents & POLLIN)
        {
            _evLoop->eventActive(_fds[i].fd, (int)FDEvent::ReadEvent);
        }
        if (_fds[i].revents & POLLOUT)
        {
            _evLoop->eventActive(_fds[i].fd, (int)FDEvent::WriteEvent);
        }
    }
    return 0;
}
