#include "HttpRequest.h"
#include <stdio.h>
#include <stdlib.h>

#define HeaderSize 12

struct HttpRequest* httpRequestInit()
{
  struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
  httpRequestReset(request);
  request->RequestHeader = (struct RequestHeader*)malloc(sizeof(struct RequestHeader) * HeaderSize);
  
  return request;
}

void httpRequestReset(HttpRequest* req)
{
  req->curState = ParseReqLine;
  req->methon = NULL;
  req->url = NULL;
  req->version = NULL;
  req->reqHeadersNum = 0;
}
