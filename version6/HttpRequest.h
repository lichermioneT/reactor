#pragma once 
#include "HttpRespone.h"
#include <stdbool.h>
#include "Buffer.h"

// 请求键值对
struct RequestHeader
{
  char* key;
  char* value;
};

// 当前的解析状态
enum HttpRequestState
{
  ParseReqLine,
  ParseReqHeaders,
  ParseReqBody,
  ParseReqDone
};

// http请求结构体
struct HttpRequest 
{
  char* methon;
  char* url;
  char* version;
  struct RequestHeader* reHeaders;
  int reqHeadersNum;
  enum HttpRequestState curState;
};

// 初始化
struct HttpRequest* httpRequsetInit();
// 重置
void httpRequestReset(struct HttpRequest* request);
void httpRequestResetEx(struct HttpRequest* request);
void httpRequestDestory(struct HttpRequest* request);

// 获取处理状态
enum HttpRequestState httpRequestState(struct HttpRequest* request);
// 添加请求头
void httRequestAddHeader(struct HttpRequest* request, const char* key, const char* values);
char* httRequestGetHeader(struct HttpRequest* request, char* key);
// 解析请求行
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* buffer); 
// 解析请求头
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* buffer,
                            struct HttpRespone* respone, struct Buffer* sendbuf, int socket); 
// 解析http请求协议
bool parseHttpRequest(struct HttpRequest* request, Buffer* buffer);
// 处理http协议
bool processHttpRequest(struct HttpRequest* request);
void decodeMsg(char* to, char* from);
int hexToDec(char c);
