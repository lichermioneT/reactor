#pragma once 

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

struct HttpRequest* httpRequsetInit();
void httpRequestReset(struct HttpRequest* request);
void httpRequestResetEx(struct HttpRequest* request);
void httpRequestDestory(struct HttpRequest* request);
