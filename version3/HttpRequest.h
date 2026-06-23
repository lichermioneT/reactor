#pragma once 

// 请求行:请求的方式，请求的静态资源，http的版本
// 请求头：键值对

enum HttpRequestState 
{
  ParseReqLine,
  ParseReqHeaders,
  ParseReqBody,
  ParseReqDone
};

struct RequestHeader 
{
  char* key;
  char* value;
};

struct HttpRequest 
{
  char* methon;
  char* url;
  char* version;
  struct RequestHeader* RequestHeader;
  int reqHeadersNum;
  enum HttpRequestState curState;
};

// 初始化
struct HttpRequest* httpRequestInit();
// 重置
void httpRequestReset(HttpRequest* req);
