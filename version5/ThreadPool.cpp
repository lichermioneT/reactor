#include "ThreadPool.h"
#include <assert.h>
#include <stdlib.h>

ThreadPool::ThreadPool(EventLoop* mainLoop, int count)
{
    _index = 0;
    _isStart = false;
    _mainLoop = mainLoop;
    _threadNum = count > 0 ? count : 0;
    _workerThreads.clear();
}

ThreadPool::~ThreadPool()
{
    for (auto item : _workerThreads)
    {
        delete item;
    }
}

void ThreadPool::run()
{
    assert(!_isStart);
    if (_mainLoop->getThreadID() != this_thread::get_id())
    {
        exit(0);
    }
    _isStart = true;
    if (_threadNum > 0)
    {
        for (int i = 0; i < _threadNum; ++i)
        {
            WorkerThread* subThread = new WorkerThread(i);
            subThread->run();
            _workerThreads.push_back(subThread);
        }
    }
}

EventLoop* ThreadPool::takeWorkerEventLoop()
{
    assert(_isStart);
    if (_mainLoop->getThreadID() != this_thread::get_id())
    {
        exit(0);
    }
    // 从线程池中找一个子线程, 然后取出里边的反应堆实例
    EventLoop* evLoop = _mainLoop;
    if (_threadNum > 0)
    {
        evLoop = _workerThreads[_index]->getEventLoop();
        _index = (_index + 1) % _threadNum;
    }
    return evLoop;
}
