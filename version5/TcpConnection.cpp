#include "TcpConnection.h"
#include "HttpRequest.h"
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"

int TcpConnection::processRead(void* arg)
{
    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    if (conn == nullptr)
    {
        return -1;
    }
    // 接收数据
    int socket = conn->_channel->getSocket();
    int count = conn->_readBuf->socketRead(socket);

    Debug("接收到的http请求数据: %.*s", conn->_readBuf->readableSize(), conn->_readBuf->data());
#ifdef MSG_SEND_AUTO
    bool needClose = count <= 0;
#endif
    if (count > 0)
    {
        // 接收到了 http 请求, 解析http请求
#ifdef MSG_SEND_AUTO
        conn->_channel->writeEventEnable(true);
        conn->_evLoop->addTask(conn->_channel, ElemType::MODIFY);
#endif
        bool flag = conn->_request->parseHttpRequest(
            conn->_readBuf, conn->_response,
            conn->_writeBuf, socket);
        if (!flag)
        {
            // 解析失败, 回复一个简单的html
            string errMsg = "HTTP/1.1 400 Bad Request\r\n\r\n";
            conn->_writeBuf->appendString(errMsg);
#ifndef MSG_SEND_AUTO
            conn->_writeBuf->sendData(socket);
#endif
        }
    }
#ifdef MSG_SEND_AUTO
    if (needClose)
    {
        // 断开连接
        conn->_evLoop->addTask(conn->_channel, ElemType::DELETE);
        return 0;
    }
#endif

#ifndef MSG_SEND_AUTO
    // 断开连接
    conn->_evLoop->addTask(conn->_channel, ElemType::DELETE);
#endif
    return 0;
}

int TcpConnection::processWrite(void* arg)
{
    Debug("开始发送数据了(基于写事件发送)....");
    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    if (conn == nullptr)
    {
        return -1;
    }
    // 发送数据
    int count = conn->_writeBuf->sendData(conn->_channel->getSocket());
    if (count > 0)
    {
        // 判断数据是否被全部发送出去了
        if (conn->_writeBuf->readableSize() == 0)
        {
            // 1. 不再检测写事件 -- 修改channel中保存的事件
            conn->_channel->writeEventEnable(false);
            // 2. 修改dispatcher检测的集合 -- 添加任务节点
            conn->_evLoop->addTask(conn->_channel, ElemType::MODIFY);
            // 3. 删除这个节点
            conn->_evLoop->addTask(conn->_channel, ElemType::DELETE);
        }
    }
    else if (count < 0)
    {
        conn->_evLoop->addTask(conn->_channel, ElemType::DELETE);
    }
    return 0;
}

int TcpConnection::destroy(void* arg)
{
    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    if (conn != nullptr)
    {
        delete conn;
    }
    return 0;
}

TcpConnection::TcpConnection(int fd, EventLoop* evloop)
{
    _evLoop = evloop;
    _readBuf = new Buffer(10240);
    _writeBuf = new Buffer(10240);
    // http
    _request = new HttpRequest;
    _response = new HttpResponse;
    _name = "Connection-" + to_string(fd);
    _channel = new Channel(fd, FDEvent::ReadEvent, processRead, processWrite, destroy, this);
    evloop->addTask(_channel, ElemType::ADD);
}

TcpConnection::~TcpConnection()
{
    delete _readBuf;
    delete _writeBuf;
    delete _request;
    delete _response;
    _readBuf = nullptr;
    _writeBuf = nullptr;
    _request = nullptr;
    _response = nullptr;

    if (_evLoop != nullptr && _channel != nullptr)
    {
        _evLoop->freeChannel(_channel);
        _channel = nullptr;
    }
    Debug("连接断开, 释放资源, gameover, connName: %s", _name.c_str());
}
