#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "EventLoop.h"
using namespace std;

// 定义子线程对应的结构体
class WorkerThread
{
public:
    WorkerThread(int index);
    ~WorkerThread();
    // 启动线程
    void run();
    inline EventLoop* getEventLoop()
    {
        return _evLoop;
    }

private:
    void running();

private:
    thread* _thread;   // 保存线程的实例
    thread::id _threadID; // ID
    string _name;
    mutex _mutex;  // 互斥锁
    condition_variable _cond;    // 条件变量
    EventLoop* _evLoop;   // 反应堆模型
};

