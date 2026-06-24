#pragma once 
#include "Buffer.h"
#include <stdbool.h>

// 请求行:请求的方式，请求的静态资源，http的版本
// 请求头：键值对

// 处理http协议的四种状态
enum HttpRequestState 
{
  ParseReqLine,
  ParseReqHeaders,
  ParseReqBody,
  ParseReqDone
};

// 请求行是键值对，定义一个键值对的结构体
struct RequestHeader 
{
  char* key;
  char* value;
};

struct HttpRequest 
{ 
  // 1.请求头的三个部分
  char* methon;
  char* url;
  char* version;
  // 2.请求行的键值对个数
  struct RequestHeader* RequestHeader;
  int reqHeadersNum;
  // 3.处理到哪个部分了
  enum HttpRequestState curState;
};

// 初始化
struct HttpRequest* httpRequestInit();
// 重置
void httpRequestReset(struct HttpRequest* req);
void httpRequestResetEx(struct HttpRequest* req);
void httpRequestDestory(struct HttpRequest* req);
// 获取处理状态
enum HttpRequestState httpRequestState(struct HttpRequest* request);
// 添加请求头
void httpRequestAddHeader(struct HttpRequest* request, char* key, char* value);
// 根据key到的value 
char* httpRequestGetHeader(struct HttpRequest* request, char* key);
// 解析请求行
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);
// 解析请求头的
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);
// 解析http协议
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf);
// 处理http请求的协议
bool processHttpRequest(struct HttpRequest* request);

void decodeMsg(char* to, char* from);
int hexToDec(char c);

