#include "TcpServer.h"
#include <arpa/inet.h>
#include "TcpConnection.h"
#include <stdio.h>
#include <stdlib.h>
#include "Log.h"

int TcpServer::acceptConnection(void* arg)
{
    TcpServer* server = static_cast<TcpServer*>(arg);
    if (server == nullptr)
    {
        return -1;
    }
    // 和客户端建立连接
    int cfd = accept(server->_lfd, NULL, NULL);
    if (cfd == -1)
    {
        perror("accept");
        return -1;
    }
    // 从线程池中取出一个子线程的反应堆实例, 去处理这个cfd
    EventLoop* evLoop = server->_threadPool->takeWorkerEventLoop();
    // 将cfd放到 TcpConnection中处理
    new TcpConnection(cfd, evLoop);
    return 0;
}

TcpServer::TcpServer(unsigned short port, int threadNum)
{
    _port = port;
    _lfd = -1;
    _mainLoop = new EventLoop;
    _threadNum = threadNum;
    _threadPool = new ThreadPool(_mainLoop, threadNum);
    setListen();
}

void TcpServer::setListen()
{
    // 1. 创建监听的fd
    _lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_lfd == -1)
    {
        perror("socket");
        return;
    }
    // 2. 设置端口复用
    int opt = 1;
    int ret = setsockopt(_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1)
    {
        perror("setsockopt");
        return;
    }
    // 3. 绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(_lfd, (struct sockaddr*)&addr, sizeof addr);
    if (ret == -1)
    {
        perror("bind");
        return ;
    }
    // 4. 设置监听
    ret = listen(_lfd, 128);
    if (ret == -1)
    {
        perror("listen");
        return ;
    }
}

void TcpServer::run()
{
    if (_lfd == -1)
    {
        return;
    }
    Debug("服务器程序已经启动了...");
    // 启动线程池
    _threadPool->run();
    // 添加检测的任务
    // 初始化一个channel实例
    Channel* channel = new Channel(_lfd, FDEvent::ReadEvent, acceptConnection, nullptr, nullptr, this);
    _mainLoop->addTask(channel, ElemType::ADD);
    // 启动反应堆模型
    _mainLoop->run();
}
