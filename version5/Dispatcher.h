#pragma once
#include "Channel.h"
#include <string>
using namespace std;
class EventLoop;
class Dispatcher
{
public:
    Dispatcher(EventLoop* evloop);
    virtual ~Dispatcher();
    // 添加
    virtual int add();
    // 删除
    virtual int remove();
    // 修改
    virtual int modify();
    // 事件监测
    virtual int dispatch(int timeout = 2); // 单位: s
    inline void setChannel(Channel* channel)
    {
        _channel = channel;
    }
protected:
    string _name = string();
    Channel* _channel = nullptr;
    EventLoop* _evLoop = nullptr;
};
