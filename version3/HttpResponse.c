#include "HttpResponse.h"
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
