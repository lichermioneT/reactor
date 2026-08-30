#include "HttpRequest.h"
#include <stdlib.h>
#include <stdio.h>

#define HeaderSize 12

struct HttpRequest* httpRequsetInit()
{
  struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
  httpRequestReset(request);
  request->reHeaders = (struct RequestHeader*)malloc(sizeof(struct RequestHeader) * HeaderSize);

  return request;
}

void httpRequestReset(struct HttpRequest* request)
{
  if(request)
  {
    request->curState  = ParseReqLine; 
    request->methon = NULL;
    request->url = NULL;
    request->version = NULL;
    request->reqHeadersNum  = 0;
  }
}

void httpRequestResetEx(struct HttpRequest* request)
{
  free(request->methon);
  free(request->url);
  free(request->version);
  httpRequestReset(request);
}

void httpRequestDestory(struct HttpRequest* request)
{
  if(request)
  {
    httpRequestResetEx(request);
    if(request->reHeaders)
    {
      for(int i = 0; i <  request->reqHeadersNum; ++i)
      {
        free(request->reHeaders[i].key);
        free(request->reHeaders[i].value);
      }
      free(request->reHeaders);
    }
    free(request);
  }
}
