#pragma once

#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

// #define MSG_SEND_AUTO

struct TcpConnection
{
  struct EventLoop* evLoop;  // 外面传进来的反应堆模型
  struct Channel* channel;   // fd注册成channel
  struct Buffer* readBuf;    // 读缓冲区
  struct Buffer* writeBuf;   // 写缓冲区
  char name[32];             // conn名称信息
  struct HttpRequest* request;   // 请求
  struct HttpRespone* response;  // 响应
};

// MODIFIED: restored TcpConnection fields and declarations.
struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evloop);
int tcpConnectionDestory(void* arg);
