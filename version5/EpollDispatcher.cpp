#include "Dispatcher.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "EpollDispatcher.h"

EpollDispatcher::EpollDispatcher(EventLoop* evloop) : Dispatcher(evloop)
{
    _epfd = epoll_create(10);
    if (_epfd == -1)
    {
        perror("epoll_create");
        exit(0);
    }
    _events = new struct epoll_event[_maxNode];
    _name = "Epoll";
}

EpollDispatcher::~EpollDispatcher()
{
    close(_epfd);
    delete[]_events;
}

int EpollDispatcher::add()
{
    int ret = epollCtl(EPOLL_CTL_ADD);
    if (ret == -1)
    {
        perror("epoll_crl add");
        exit(0);
    }
    return ret;
}

int EpollDispatcher::remove()
{
    int ret = epollCtl(EPOLL_CTL_DEL);
    if (ret == -1)
    {
        perror("epoll_crl delete");
        exit(0);
    }
    // 通过 channel 释放对应的 TcpConnection 资源
    if (_channel->_destroyCallback)
    {
        _channel->_destroyCallback(const_cast<void*>(_channel->getArg()));
    }

    return ret;
}

int EpollDispatcher::modify()
{
    int ret = epollCtl(EPOLL_CTL_MOD);
    if (ret == -1)
    {
        perror("epoll_crl modify");
        exit(0);
    }
    return ret;
}

int EpollDispatcher::dispatch(int timeout)
{
    int count = epoll_wait(_epfd, _events, _maxNode, timeout * 1000);
    if (count == -1)
    {
        perror("epoll_wait");
        exit(0);
    }
    for (int i = 0; i < count; ++i)
    {
        int events = _events[i].events;
        int fd = _events[i].data.fd;
        if ((events & EPOLLERR) || (events & EPOLLHUP))
        {
            // 对方断开了连接, 删除 fd
            // epollRemove(Channel, evLoop);
            continue;
        }
        if (events & EPOLLIN)
        {
            _evLoop->eventActive(fd, (int)FDEvent::ReadEvent);
        }
        if (events & EPOLLOUT)
        {
            _evLoop->eventActive(fd, (int)FDEvent::WriteEvent);
        }
    }
    return 0;
}

int EpollDispatcher::epollCtl(int op)
{
    struct epoll_event ev;
    ev.data.fd = _channel->getSocket();
    int events = 0;
    if (_channel->getEvent() & (int)FDEvent::ReadEvent)
    {
        events |= EPOLLIN;
    }
    if (_channel->getEvent() & (int)FDEvent::WriteEvent)
    {
        events |= EPOLLOUT;
    }
    ev.events = events;
    int ret = epoll_ctl(_epfd, op, _channel->getSocket(), &ev);
    return ret;
}
