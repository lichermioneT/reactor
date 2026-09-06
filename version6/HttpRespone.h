#pragma once 
#include "Buffer.h"

// 定义常见的状态码
enum HttpStatusCode
{
  Unknown,
  OK = 200,
  MovedPermanently = 301,
  MovedTemporaily = 302,
  BadRequest = 400,
  NotFound = 404
};

// 响应的结构体
struct HttpResponeHeader
{
 char key[32];
 char value[128];
};

// 定义一个函数指针，用来组织需要回复给客户端的数据
typedef void(*responeBody)(const char* fileName, struct Buffer* sendBbuf, int socket);

struct HttpRespone
{
// 状态行:状态码，状态描述
  enum HttpStatusCode statusCode;
  char statusMsg[128];

  char fileName[128];

// 响应头--键值对
  struct HttpResponeHeader* header;
  int headerNum;
// 回复的数据
  responeBody sendDataFunc;

};

// 初始化
struct HttpRespone* httResponeInit();
// 销毁函数
void httpResponeDestory(struct HttpRespone* respone);
// 添加响应头
void httpResponeAddHeader(struct HttpRespone* respone, const char* key, const char* value);
// 组织http响应数据
void httpResponePrepareMsg(struct HttpRespone* respone, struct Buffer* sendBuf, int socket);
