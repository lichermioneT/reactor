#include "HttpRespone.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#define ResHeaderSize 16

struct HttpRespone* httResponeInit()
{
  struct HttpRespone* httpRespone = (struct HttpRespone*)malloc(sizeof(struct HttpRespone));
  httpRespone->headerNum = 0;
  int size = sizeof(struct HttpResponeHeader) * ResHeaderSize; 
  httpRespone->header = (struct HttpResponeHeader*)malloc(size);
  httpRespone->statusCode = Unknown; 
  bzero(httpRespone->header, size);
  bzero(httpRespone->statusMsg, sizeof(httpRespone->statusMsg));
  bzero(httpRespone->fileName, sizeof(httpRespone->fileName));
  httpRespone->sendDataFunc = NULL;

  return httpRespone;
}

void httpResponeDestory(struct HttpRespone* respone)
{
  if(respone)
  {
    free(respone->header);
    free(respone);
  }
}

void httpResponeAddHeader(struct HttpRespone* respone, const char* key, const char* value)
{
  if(respone || key || value)
  {
    return;
  }

  strcpy(respone->header[respone->headerNum].key, key);
  strcpy(respone->header[respone->headerNum].value,value);
  respone->headerNum++;
}

void httpResponePrepareMsg(struct HttpRespone* respone, struct Buffer* sendBuf, int socket)
{
  // 状态行
  char tmp[1024] = {0};
  sprintf(tmp, "HTTP/1.1 %d %s\r\n", respone->statusCode, respone->statusMsg);
  bufferAppendString(sendBuf, tmp);
  // 响应头
  for(int i = 0; i < respone->headerNum; ++i)
  {
    sprintf(tmp, "%s: %s\r\n", respone->header[i].key, respone->header[i].value);
    bufferAppendString(sendBuf, tmp);
  }
  // 空行
  bufferAppendString(sendBuf, "\r\n");
  // 回复的数据
  respone->sendDataFunc(respone->fileName, sendBuf, socket);
}
