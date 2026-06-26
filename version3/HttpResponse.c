#include "HttpResponse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define ResHeaderSize 16

struct HttpRespone* httResponseInit()
{
  struct HttpRespone* respone = (struct HttpRespone*)malloc(sizeof(struct HttpRespone));
  respone->headerNum = 0;
  
  int size = sizeof(struct ResponseHeader) * ResHeaderSize; 
  respone->headers = (struct ResponseHeader*)malloc(size);
  respone->statusCode = Unkown;

  bzero(respone->headers, size);
  bzero(respone->statusMsg, sizeof(respone->statusMsg));
  bzero(respone->fileName, sizeof(respone->fileName));

  respone->sendDataFunc = NULL;

  return respone;
}

void  httpResponeDestory(struct HttpRespone* respone)
{
  if(respone != NULL)
  {
    free(respone->headers);
    free(respone);
    respone = NULL;
  }
}

void httpsResponeAddHeader(struct HttpRespone* respone, const char* key, const char* value)
{
  if(respone != NULL || key != NULL || value != NULL)
  {
    return;
  }

  strcpy(respone->headers[respone->headerNum].key,  key);
  strcpy(respone->headers[respone->headerNum].value, key);
  respone->headerNum++;
}

void httpResponsePrepareMsg(struct HttpRespone* respone, struct Buffer* sendBuf, int socket)
{
  // 状态行
  char tmp[1024] = {0};
  sprintf(tmp, "HTTP/1.1 %d %s\r\n", respone->statusCode, respone->statusMsg);
  bufferAppendString(sendBuf, tmp);

  // 响应头
  for(int i = 0; i < respone->headerNum; ++i)
  {
    sprintf(tmp, "%s: %s\r\n", respone->headers[i].key, respone->headers[i].value);
  }
  bufferAppendString(sendBuf, tmp);

  // 空行
  bufferAppendString(sendBuf, "\r\n");
  // 回复的数据
  respone->sendDataFunc(respone->fileName, sendBuf, socket);
}
