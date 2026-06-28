#define _GNU_SOURCE

#include "HttpRequest.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define HeaderSize 12

static char* splitRequestLine(char* start, char* end, const char* sub, char** ptr)
{
  char* space = end;
  if(sub != NULL)
  {
    space = (char*)memmem(start, (size_t)(end - start), sub, strlen(sub));
    if(space == NULL)
    {
      return NULL;
    }
  }

  int length = (int)(space - start);
  char* temp = (char*)malloc((size_t)length + 1);
  if(temp == NULL)
  {
    perror("malloc");
    return NULL;
  }

  memcpy(temp, start, (size_t)length);
  temp[length] = '\0';
  *ptr = temp;

  return sub == NULL ? space : space + strlen(sub);
}

static void setSimpleResponse(struct HttpRespone* respone,
                              enum HttpStatusCode code,
                              const char* statusMsg)
{
  httpResponeReset(respone);
  respone->statusCode = code;
  snprintf(respone->statusMsg, sizeof(respone->statusMsg), "%s", statusMsg);
  httpsResponeAddHeader(respone, "Content-length", "0");
}

struct HttpRequest* httpRequestInit()
{
  struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
  if(request == NULL)
  {
    perror("malloc");
    return NULL;
  }

  memset(request, 0, sizeof(struct HttpRequest));
  request->RequestHeader =
      (struct RequestHeader*)calloc(HeaderSize, sizeof(struct RequestHeader));
  if(request->RequestHeader == NULL)
  {
    perror("calloc");
    free(request);
    return NULL;
  }

  httpRequestReset(request);
  return request;
}

void httpRequestReset(struct HttpRequest* req)
{
  if(req == NULL)
  {
    return;
  }

  req->curState = ParseReqLine;
  req->methon = NULL;
  req->url = NULL;
  req->version = NULL;
  req->reqHeadersNum = 0;
}

void httpRequestResetEx(struct HttpRequest* req)
{
  if(req == NULL)
  {
    return;
  }

  free(req->methon);
  free(req->url);
  free(req->version);

  for(int i = 0; i < req->reqHeadersNum; ++i)
  {
    free(req->RequestHeader[i].key);
    free(req->RequestHeader[i].value);
    req->RequestHeader[i].key = NULL;
    req->RequestHeader[i].value = NULL;
  }

  httpRequestReset(req);
}

void httpRequestDestory(struct HttpRequest* req)
{
  if(req != NULL)
  {
    httpRequestResetEx(req);
    free(req->RequestHeader);
    free(req);
  }
}

enum HttpRequestState httpRequestState(struct HttpRequest* request)
{
  return request == NULL ? ParseReqLine : request->curState;
}

void httpRequestAddHeader(struct HttpRequest* request, char* key, char* value)
{
  if(request == NULL || key == NULL || value == NULL ||
     request->reqHeadersNum >= HeaderSize)
  {
    free(key);
    free(value);
    return;
  }

  // MODIFIED: append to the current header slot instead of overwriting slot 0.
  request->RequestHeader[request->reqHeadersNum].key = key;
  request->RequestHeader[request->reqHeadersNum].value = value;
  request->reqHeadersNum++;
}

char* httpRequestGetHeader(struct HttpRequest* request, char* key)
{
  if(request != NULL && key != NULL)
  {
    for(int i = 0; i < request->reqHeadersNum; ++i)
    {
      if(strcasecmp(request->RequestHeader[i].key, key) == 0)
      {
        return request->RequestHeader[i].value;
      }
    }
  }

  return NULL;
}

bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf)
{
  char* end = bufferFindCRLF(readBuf);
  if(request == NULL || readBuf == NULL || end == NULL)
  {
    return false;
  }

  char* start = readBuf->data + readBuf->readPos;
  int len = (int)(end - start);
  if(len <= 0)
  {
    return false;
  }

  start = splitRequestLine(start, end, " ", &request->methon);
  if(start == NULL)
  {
    return false;
  }

  start = splitRequestLine(start, end, " ", &request->url);
  if(start == NULL)
  {
    return false;
  }

  if(splitRequestLine(start, end, NULL, &request->version) == NULL)
  {
    return false;
  }

  // MODIFIED: advance the buffer and move parser state after parsing request line.
  readBuf->readPos += len + 2;
  request->curState = ParseReqHeaders;
  return true;
}

bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf)
{
  char* end = bufferFindCRLF(readBuf);
  if(request == NULL || readBuf == NULL || end == NULL)
  {
    return false;
  }

  char* start = readBuf->data + readBuf->readPos;
  int lineSize = (int)(end - start);
  char* middle = (char*)memmem(start, (size_t)lineSize, ": ", 2);

  if(middle != NULL)
  {
    char* key = (char*)malloc((size_t)(middle - start) + 1);
    char* value = (char*)malloc((size_t)(end - middle - 2) + 1);
    if(key == NULL || value == NULL)
    {
      free(key);
      free(value);
      return false;
    }

    memcpy(key, start, (size_t)(middle - start));
    key[middle - start] = '\0';
    memcpy(value, middle + 2, (size_t)(end - middle - 2));
    value[end - middle - 2] = '\0';

    httpRequestAddHeader(request, key, value);
    readBuf->readPos += lineSize + 2;
  }
  else
  {
    readBuf->readPos += 2;
    request->curState = ParseReqDone;
  }

  return true;
}

bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,
                      struct HttpRespone* respone, struct Buffer* sendBuf,
                      int socket)
{
  if(request == NULL || readBuf == NULL || respone == NULL || sendBuf == NULL)
  {
    return false;
  }

  bool flag = true;
  while(request->curState != ParseReqDone)
  {
    switch(request->curState)
    {
    case ParseReqLine:
      flag = parseHttpRequestLine(request, readBuf);
      break;
    case ParseReqHeaders:
      flag = parseHttpRequestHeader(request, readBuf);
      break;
    case ParseReqBody:
      request->curState = ParseReqDone;
      break;
    default:
      flag = false;
      break;
    }

    if(!flag)
    {
      return false;
    }
  }

  if(!processHttpRequest(request, respone))
  {
    return false;
  }

  httpResponsePrepareMsg(respone, sendBuf, socket);
  httpRequestResetEx(request);
  return true;
}

bool processHttpRequest(struct HttpRequest* request, struct HttpRespone* respone)
{
  if(request == NULL || respone == NULL || request->methon == NULL ||
     request->url == NULL)
  {
    return false;
  }

  httpResponeReset(respone);

  if(strcasecmp("get", request->methon) != 0)
  {
    setSimpleResponse(respone, BadRequest, "Bad Request");
    return true;
  }

  decodeMsg(request->url, request->url);
  if(strstr(request->url, "..") != NULL)
  {
    setSimpleResponse(respone, BadRequest, "Bad Request");
    return true;
  }

  const char* file = strcmp(request->url, "/") == 0 ? "./" : request->url + 1;

  struct stat st;
  int ret = stat(file, &st);
  if(ret == -1)
  {
    struct stat errorPage;
    respone->statusCode = NotFound;
    snprintf(respone->statusMsg, sizeof(respone->statusMsg), "Not Found");

    if(stat("404.html", &errorPage) == 0 && S_ISREG(errorPage.st_mode))
    {
      snprintf(respone->fileName, sizeof(respone->fileName), "%s", "404.html");
      httpsResponeAddHeader(respone, "Content-type", getFileType(".html"));

      char len[32] = {0};
      snprintf(len, sizeof(len), "%ld", (long)errorPage.st_size);
      httpsResponeAddHeader(respone, "Content-length", len);
      respone->sendDataFunc = sendFile;
    }
    else
    {
      httpsResponeAddHeader(respone, "Content-length", "0");
    }

    return true;
  }

  snprintf(respone->fileName, sizeof(respone->fileName), "%s", file);
  respone->statusCode = OK;
  snprintf(respone->statusMsg, sizeof(respone->statusMsg), "OK");

  if(S_ISDIR(st.st_mode))
  {
    httpsResponeAddHeader(respone, "Content-type", getFileType(".html"));
    respone->sendDataFunc = sendDir;
  }
  else
  {
    char tmp[32] = {0};
    snprintf(tmp, sizeof(tmp), "%ld", (long)st.st_size);

    // MODIFIED: use the requested file for MIME type and raw size for length.
    httpsResponeAddHeader(respone, "Content-type", getFileType(file));
    httpsResponeAddHeader(respone, "Content-length", tmp);
    respone->sendDataFunc = sendFile;
  }

  return true;
}

void decodeMsg(char* to, char* from)
{
  for(; *from != '\0'; ++to, ++from)
  {
    if(from[0] == '%' && isxdigit((unsigned char)from[1]) &&
       isxdigit((unsigned char)from[2]))
    {
      *to = (char)(hexToDec(from[1]) * 16 + hexToDec(from[2]));
      from += 2;
    }
    else
    {
      *to = *from;
    }
  }
  *to = '\0';
}

int hexToDec(char c)
{
  if(c >= '0' && c <= '9')
  {
    return c - '0';
  }
  if(c >= 'a' && c <= 'f')
  {
    return c - 'a' + 10;
  }
  if(c >= 'A' && c <= 'F')
  {
    return c - 'A' + 10;
  }

  return 0;
}

const char* getFileType(const char* name)
{
  const char* dot = strrchr(name, '.');
  if(dot == NULL)
  {
    return "text/plain; charset=utf-8";
  }
  if(strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
  {
    return "text/html; charset=utf-8";
  }
  if(strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
  {
    return "image/jpeg";
  }
  if(strcmp(dot, ".gif") == 0)
  {
    return "image/gif";
  }
  if(strcmp(dot, ".png") == 0)
  {
    return "image/png";
  }
  if(strcmp(dot, ".css") == 0)
  {
    return "text/css";
  }
  if(strcmp(dot, ".js") == 0)
  {
    return "application/javascript";
  }
  if(strcmp(dot, ".json") == 0)
  {
    return "application/json";
  }
  if(strcmp(dot, ".md") == 0 || strcmp(dot, ".markdown") == 0)
  {
    return "text/markdown; charset=utf-8";
  }
  if(strcmp(dot, ".pdf") == 0)
  {
    return "application/pdf";
  }
  if(strcmp(dot, ".wav") == 0)
  {
    return "audio/wav";
  }
  if(strcmp(dot, ".mp3") == 0)
  {
    return "audio/mpeg";
  }
  if(strcmp(dot, ".mp4") == 0)
  {
    return "video/mp4";
  }

  return "text/plain; charset=utf-8";
}

int sendDir(const char* dirName, struct Buffer* sendBuf, int cfd)
{
  char buf[4096] = {0};
  snprintf(buf, sizeof(buf), "<html><head><title>%s</title></head><body><table>",
           dirName);
  bufferAppendString(sendBuf, buf);

  struct dirent** namelist = NULL;
  int num = scandir(dirName, &namelist, NULL, alphasort);
  if(num < 0)
  {
    perror("scandir");
    return -1;
  }

  for(int i = 0; i < num; ++i)
  {
    char* name = namelist[i]->d_name;
    struct stat st;
    char subPath[1024] = {0};
    snprintf(subPath, sizeof(subPath), "%s/%s", dirName, name);

    if(stat(subPath, &st) == -1)
    {
      free(namelist[i]);
      continue;
    }

    if(S_ISDIR(st.st_mode))
    {
      snprintf(buf, sizeof(buf),
               "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
               name, name, (long)st.st_size);
    }
    else
    {
      snprintf(buf, sizeof(buf),
               "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
               name, name, (long)st.st_size);
    }

    bufferAppendString(sendBuf, buf);
#ifndef MSG_SEND_AUTO
    bufferSendData(sendBuf, cfd);
#endif
    free(namelist[i]);
  }

  snprintf(buf, sizeof(buf), "</table></body></html>");
  bufferAppendString(sendBuf, buf);
#ifndef MSG_SEND_AUTO
  bufferSendData(sendBuf, cfd);
#endif

  free(namelist);
  return 0;
}

int sendFile(const char* fileName, struct Buffer* sendBuf, int cfd)
{
  int fd = open(fileName, O_RDONLY);
  if(fd < 0)
  {
    perror("open");
    return -1;
  }

  while(1)
  {
    char buf[1024];
    int len = (int)read(fd, buf, sizeof(buf));
    if(len > 0)
    {
      bufferAppendData(sendBuf, buf, len);
#ifndef MSG_SEND_AUTO
      bufferSendData(sendBuf, cfd);
#endif
      usleep(10);
    }
    else if(len == 0)
    {
      break;
    }
    else
    {
      perror("read");
      break;
    }
  }

  close(fd);
  return 0;
}
