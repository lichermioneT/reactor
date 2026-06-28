#pragma once

#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

// #define MSG_SEND_AUTO

struct TcpConnection
{
  struct EventLoop* evLoop;
  struct Channel* channel;
  struct Buffer* readBuf;
  struct Buffer* writeBuf;
  char name[32];
  struct HttpRequest* request;
  struct HttpRespone* response;
};

// MODIFIED: restored TcpConnection fields and declarations.
struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evloop);
int tcpConnectionDestory(void* arg);
