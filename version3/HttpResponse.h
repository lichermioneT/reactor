#pragma once 
#include "Buffer.h"

enum HttpStatusCode 
{
  Unkown,
  OK = 200,
  MovePermanently = 30l,
  MoveTemporarily = 302,
  BadRequest = 400,
  NotFound = 404
};

struct ResponseHeader 
{
  char key[32];
  char value[128];
};

typedef void(*responseBody)(const char* fileName, struct Buffer* snedBuf, int socket);

struct HttpRespone 
{
// 状态行：状态码 状态描述 版本
  enum HttpStatusCode statusCode;
  char statusMsg[128];
// 响应头：键值对
  struct ResponseHeader* headers;
  int headerNum;
  responseBody sendDataFunc;
};

// 初始化
struct HttpRespone* httResponseInit();
// 销毁
void  httpResponeDestory(struct HttpRespone* respone);
// 添加响应头
void httpsResponeAddHeader(struct HttpRespone* respone, const char* key, const char* value);

