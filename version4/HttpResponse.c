#include "HttpResponse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ResHeaderSize 16

struct HttpRespone* httResponseInit()
{
  struct HttpRespone* respone = (struct HttpRespone*)malloc(sizeof(struct HttpRespone));
  if(respone == NULL)
  {
    perror("malloc");
    return NULL;
  }

  respone->headers =
      (struct ResponseHeader*)calloc(ResHeaderSize, sizeof(struct ResponseHeader));
  if(respone->headers == NULL)
  {
    perror("calloc");
    free(respone);
    return NULL;
  }

  httpResponeReset(respone);
  return respone;
}

void httpResponeReset(struct HttpRespone* respone)
{
  if(respone == NULL)
  {
    return;
  }

  respone->headerNum = 0;
  respone->statusCode = Unkown;
  memset(respone->statusMsg, 0, sizeof(respone->statusMsg));
  memset(respone->fileName, 0, sizeof(respone->fileName));
  memset(respone->headers, 0, sizeof(struct ResponseHeader) * ResHeaderSize);
  respone->sendDataFunc = NULL;
}

void httpResponeDestory(struct HttpRespone* respone)
{
  if(respone != NULL)
  {
    free(respone->headers);
    free(respone);
  }
}

void httpsResponeAddHeader(struct HttpRespone* respone, const char* key,
                           const char* value)
{
  // MODIFIED: original condition returned for valid input and copied key twice.
  if(respone == NULL || key == NULL || value == NULL ||
     respone->headerNum >= ResHeaderSize)
  {
    return;
  }

  snprintf(respone->headers[respone->headerNum].key,
           sizeof(respone->headers[respone->headerNum].key), "%s", key);
  snprintf(respone->headers[respone->headerNum].value,
           sizeof(respone->headers[respone->headerNum].value), "%s", value);
  respone->headerNum++;
}

void httpResponsePrepareMsg(struct HttpRespone* respone, struct Buffer* sendBuf,
                            int socket)
{
  if(respone == NULL || sendBuf == NULL)
  {
    return;
  }

  char tmp[1024] = {0};
  snprintf(tmp, sizeof(tmp), "HTTP/1.1 %d %s\r\n",
           respone->statusCode, respone->statusMsg);
  bufferAppendString(sendBuf, tmp);

  for(int i = 0; i < respone->headerNum; ++i)
  {
    snprintf(tmp, sizeof(tmp), "%s: %s\r\n",
             respone->headers[i].key, respone->headers[i].value);
    bufferAppendString(sendBuf, tmp);
  }

  bufferAppendString(sendBuf, "\r\n");

  if(respone->sendDataFunc != NULL)
  {
    respone->sendDataFunc(respone->fileName, sendBuf, socket);
  }

#ifndef MSG_SEND_AUTO
  bufferSendData(sendBuf, socket);
#endif
}
