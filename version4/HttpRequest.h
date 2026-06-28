#pragma once

#include "Buffer.h"
#include "HttpResponse.h"

#include <stdbool.h>

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

// MODIFIED: restored request enum/API declarations that were commented out.
struct HttpRequest* httpRequestInit();
void httpRequestReset(struct HttpRequest* req);
void httpRequestResetEx(struct HttpRequest* req);
void httpRequestDestory(struct HttpRequest* req);
enum HttpRequestState httpRequestState(struct HttpRequest* request);
void httpRequestAddHeader(struct HttpRequest* request, char* key, char* value);
char* httpRequestGetHeader(struct HttpRequest* request, char* key);
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,
                      struct HttpRespone* respone, struct Buffer* sendBuf,
                      int socket);
bool processHttpRequest(struct HttpRequest* request, struct HttpRespone* respone);

void decodeMsg(char* to, char* from);
int hexToDec(char c);
const char* getFileType(const char* name);
int sendDir(const char* dirName, struct Buffer* buffer, int cfd);
int sendFile(const char* fileName, struct Buffer* buffer, int cfd);
