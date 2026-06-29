#include "WorkerThread.h"
#include <stdio.h>

// 子线程的回调函数
void WorkerThread::running()
{
    _mutex.lock();
    _threadID = this_thread::get_id();
    _evLoop = new EventLoop(_name);
    _mutex.unlock();
    _cond.notify_one();
    _evLoop->run();
}

WorkerThread::WorkerThread(int index)
{
    _evLoop = nullptr;
    _thread = nullptr;
    _threadID = thread::id();
    _name =  "SubThread-" + to_string(index);
}

WorkerThread::~WorkerThread()
{
    if (_thread != nullptr)
    {
        if (_thread->joinable())
        {
            _thread->detach();
        }
        delete _thread;
    }
}

void WorkerThread::run()
{
    // 创建子线程
    _thread = new thread(&WorkerThread::running, this);
    // 阻塞主线程, 让当前函数不会直接结束
    unique_lock<mutex> locker(_mutex);
    while (_evLoop == nullptr)
    {
        _cond.wait(locker);
    }
}
