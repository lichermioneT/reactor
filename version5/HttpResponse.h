#pragma once

#include "Buffer.h"

enum HttpStatusCode
{
  Unkown,
  OK = 200,
  MovePermanently = 301,
  MoveTemporarily = 302,
  BadRequest = 400,
  NotFound = 404
};

struct ResponseHeader
{
  char key[32];
  char value[128];
};

typedef int (*responseBody)(const char* fileName, struct Buffer* sendBuf, int socket);

struct HttpRespone
{
  enum HttpStatusCode statusCode;
  char statusMsg[128];
  char fileName[128];
  struct ResponseHeader* headers;
  int headerNum;
  responseBody sendDataFunc;
};

// MODIFIED: restored response declarations.
struct HttpRespone* httResponseInit();
void httpResponeDestory(struct HttpRespone* respone);
void httpResponeReset(struct HttpRespone* respone);
void httpsResponeAddHeader(struct HttpRespone* respone, const char* key,
                           const char* value);
void httpResponsePrepareMsg(struct HttpRespone* respone, struct Buffer* sendBuf,
                            int socket);
