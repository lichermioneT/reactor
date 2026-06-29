#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

class TcpServer
{
public:
    TcpServer(unsigned short port, int threadNum);
    // 初始化监听
    void setListen();
    // 启动服务器
    void run();
    static int acceptConnection(void* arg);

private:
    int _threadNum;
    EventLoop* _mainLoop;
    ThreadPool* _threadPool;
    int _lfd;
    unsigned short _port;
};

