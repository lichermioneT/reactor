#include "EventLoop.h"
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include "SelectDispatcher.h"
#include "PollDispatcher.h"
#include "EpollDispatcher.h"

EventLoop::EventLoop() : EventLoop(string())
{
}

EventLoop::EventLoop(const string threadName)
{
    _isQuit = true;    // 默认没有启动
    _threadID = this_thread::get_id();
    _threadName = threadName == string() ? "MainThread" : threadName;
    _dispatcher = new SelectDispatcher(this);
    // map
    _channelMap.clear();
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, _socketPair);
    if (ret == -1)
    {
        perror("socketpair");
        exit(0);
    }
#if 0
    // 指定规则: evLoop->socketPair[0] 发送数据, evLoop->socketPair[1] 接收数据
    Channel* channel = new Channel(_socketPair[1], FDEvent::ReadEvent,
        readLocalMessage, nullptr, nullptr, this);
#else
    // 绑定 - bind
    auto obj = bind(&EventLoop::readMessage, this);
    Channel* channel = new Channel(_socketPair[1], FDEvent::ReadEvent,
        obj, nullptr, nullptr, this);
#endif
    // channel 添加到任务队列
    addTask(channel, ElemType::ADD);
}

EventLoop::~EventLoop()
{
    delete _dispatcher;
    _dispatcher = nullptr;
    close(_socketPair[0]);
    close(_socketPair[1]);
    while (!_taskQ.empty())
    {
        ChannelElement* node = _taskQ.front();
        _taskQ.pop();
        delete node;
    }
}

int EventLoop::run()
{
    _isQuit = false;
    // 比较线程ID是否正常
    if (_threadID != this_thread::get_id())
    {
        return -1;
    }
    // 循环进行事件处理
    while (!_isQuit)
    {
        _dispatcher->dispatch();    // 超时时长 2s
        processTaskQ();
    }
    return 0;
}

int EventLoop::eventActive(int fd, int event)
{
    if (fd < 0)
    {
        return -1;
    }
    // 取出channel
    auto item = _channelMap.find(fd);
    if (item == _channelMap.end() || item->second == nullptr)
    {
        return -1;
    }
    Channel* channel = item->second;
    assert(channel->getSocket() == fd);
    if (event & (int)FDEvent::ReadEvent && channel->_readCallback)
    {
        channel->_readCallback(const_cast<void*>(channel->getArg()));
    }
    if (event & (int)FDEvent::WriteEvent && channel->_writeCallback)
    {
        channel->_writeCallback(const_cast<void*>(channel->getArg()));
    }
    return 0;
}

int EventLoop::addTask(Channel* channel, ElemType type)
{
    ChannelElement* node = new ChannelElement;
    node->_channel = channel;
    node->_type = type;
    {
        lock_guard<mutex> locker(_mutex);
        _taskQ.push(node);
    }
    // 处理节点
    /*
    * 细节:
    *   1. 对于链表节点的添加: 可能是当前线程也可能是其他线程(主线程)
    *       1). 修改fd的事件, 当前子线程发起, 当前子线程处理
    *       2). 添加新的fd, 添加任务节点的操作是由主线程发起的
    *   2. 不能让主线程处理任务队列, 需要由当前的子线程取处理
    */
    if (_threadID == this_thread::get_id())
    {
        // 当前子线程(基于子线程的角度分析)
        processTaskQ();
    }
    else
    {
        // 主线程 -- 告诉子线程处理任务队列中的任务
        // 1. 子线程在工作 2. 子线程被阻塞了:select, poll, epoll
        taskWakeup();
    }
    return 0;
}

int EventLoop::processTaskQ()
{
    // 取出头结点
    while (true)
    {
        ChannelElement* node = nullptr;
        {
            lock_guard<mutex> locker(_mutex);
            if (_taskQ.empty())
            {
                break;
            }
            node = _taskQ.front();
            _taskQ.pop();  // 删除节点
        }
        Channel* channel = node->_channel;
        if (node->_type == ElemType::ADD)
        {
            // 添加
            add(channel);
        }
        else if (node->_type == ElemType::DELETE)
        {
            // 删除
            remove(channel);
        }
        else if (node->_type == ElemType::MODIFY)
        {
            // 修改
            modify(channel);
        }
        delete node;
    }
    return 0;
}

int EventLoop::add(Channel* channel)
{
    if (channel == nullptr)
    {
        return -1;
    }
    int fd = channel->getSocket();
    // 找到fd对应的数组元素位置, 并存储
    if (_channelMap.find(fd) == _channelMap.end())
    {
        _channelMap.insert(make_pair(fd, channel));
        _dispatcher->setChannel(channel);
        int ret = _dispatcher->add();
        return ret;
    }
    return -1;
}

int EventLoop::remove(Channel* channel)
{
    if (channel == nullptr)
    {
        return -1;
    }
    int fd = channel->getSocket();
    if (_channelMap.find(fd) == _channelMap.end())
    {
        return -1;
    }
    _dispatcher->setChannel(channel);
    int ret = _dispatcher->remove();
    return ret;
}

int EventLoop::modify(Channel* channel)
{
    if (channel == nullptr)
    {
        return -1;
    }
    int fd = channel->getSocket();
    if (_channelMap.find(fd) == _channelMap.end())
    {
        return -1;
    }
    _dispatcher->setChannel(channel);
    int ret = _dispatcher->modify();
    return ret;
}

int EventLoop::readLocalMessage(void* arg)
{
    EventLoop* evLoop = static_cast<EventLoop*>(arg);
    char buf[256];
    read(evLoop->_socketPair[1], buf, sizeof(buf));
    return 0;
}

void EventLoop::taskWakeup()
{
    const char* msg = "我是要成为海贼王的男人!!!";
    write(_socketPair[0], msg, strlen(msg));
}

int EventLoop::freeChannel(Channel* channel)
{
    if (channel == nullptr)
    {
        return -1;
    }
    // 删除 channel 和 fd 的对应关系
    auto it = _channelMap.find(channel->getSocket());
    if (it != _channelMap.end())
    {
        _channelMap.erase(it);
        close(channel->getSocket());
        delete channel;
    }
    return 0;
}

int EventLoop::readMessage()
{
    char buf[256];
    read(_socketPair[1], buf, sizeof(buf));
    return 0;
}
