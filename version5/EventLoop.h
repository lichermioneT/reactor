#pragma once
#include "Channel.h"
#include <string>
#include <thread>
#include <queue>
#include <map>
#include <mutex>
using namespace std;

// 处理该节点中的channel的方式
enum class ElemType:char{ADD, DELETE, MODIFY};
// 定义任务队列的节点
struct ChannelElement
{
    ElemType _type;   // 如何处理该节点中的channel
    Channel* _channel;
};
class Dispatcher;

class EventLoop
{
public:
    EventLoop();
    EventLoop(const string threadName);
    ~EventLoop();
    // 启动反应堆模型
    int run();
    // 处理别激活的文件fd
    int eventActive(int fd, int event);
    // 添加任务到任务队列
    int addTask(struct Channel* channel, ElemType type);
    // 处理任务队列中的任务
    int processTaskQ();
    // 处理dispatcher中的节点
    int add(Channel* channel);
    int remove(Channel* channel);
    int modify(Channel* channel);
    // 释放channel
    int freeChannel(Channel* channel);
    int readMessage();
    // 返回线程ID
    inline thread::id getThreadID()
    {
        return _threadID;
    }
    inline string getThreadName()
    {
        return _threadName;
    }
    static int readLocalMessage(void* arg);

private:
    void taskWakeup();

private:
    bool _isQuit;
    // 该指针指向子类的实例 epoll, poll, select
    Dispatcher* _dispatcher;
    // 任务队列
    queue<ChannelElement*> _taskQ;
    // map
    map<int, Channel*> _channelMap;
    // 线程id, name, mutex
    thread::id _threadID;
    string _threadName;
    mutex _mutex;
    int _socketPair[2];  // 存储本地通信的fd 通过socketpair 初始化
};


