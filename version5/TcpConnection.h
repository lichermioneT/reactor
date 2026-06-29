#pragma once
#include "EventLoop.h"
#include "Buffer.h"
#include "Channel.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

//#define MSG_SEND_AUTO

class TcpConnection
{
public:
    TcpConnection(int fd, EventLoop* evloop);
    ~TcpConnection();

    static int processRead(void* arg);
    static int processWrite(void* arg);
    static int destroy(void* arg);
private:
    string _name;
    EventLoop* _evLoop;
    Channel* _channel;
    Buffer* _readBuf;
    Buffer* _writeBuf;
    // http 协议
    HttpRequest* _request;
    HttpResponse* _response;
};