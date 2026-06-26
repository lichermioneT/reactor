#include "HttpRequest.h"
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "HttpResponse.h"
#include <sys/stat.h>
#include <ctype.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HeaderSize 12

struct HttpRequest* httpRequestInit()
{
  // 1.开空间
  struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
  // 2.全部初始化
  httpRequestReset(request);
  // 3.给键值对开辟空间的
  request->RequestHeader = (struct RequestHeader*)malloc(sizeof(struct RequestHeader) * HeaderSize);
  
  return request;
}

void httpRequestReset(struct HttpRequest* req)
{
  req->curState = ParseReqLine;
  req->methon = NULL;
  req->url = NULL;
  req->version = NULL;
  req->reqHeadersNum = 0;
}

void httpRequestResetEx(struct HttpRequest* req)
{
  free(req->methon);
  free(req->url);
  free(req->version);
  if(req->RequestHeader != NULL)
  {
    for(int i  = 0; i < req->reqHeadersNum; ++i)
    {
      free(req->RequestHeader[i].key);
      free(req->RequestHeader[i].value);
    }
    free(req->RequestHeader);
  }
  httpRequestReset(req);
}

void httpRequestDestory(struct HttpRequest* req)
{
  if(req != NULL)
  {
    httpRequestResetEx(req); 
    free(req);
  }
}

enum HttpRequestState httpRequestState(struct HttpRequest* request)
{
  return request->curState;
}

void httpRequestAddHeader(struct HttpRequest* request, char* key, char* value)
{
  request->RequestHeader->key = key;
  request->RequestHeader->value = value;
  request->reqHeadersNum++;
}

char* httpRequestGetHeader(struct HttpRequest* request, char* key)
{
  if(request != NULL)
  {
    for(int i = 0; i < request->reqHeadersNum; ++i)
    {
      if(strncasecmp(request->RequestHeader[i].key, key, strlen(key)) == 0)
      {
        return request->RequestHeader[i].value;
      }
    }
  }

  return NULL;
}

char* spliteRequestLine(char* start, char* end, char* sub, char** ptr)
{
  char* space = end;
  if(sub != NULL)
  {
    space = (char*)memmem(start, end - start, sub, strlen(sub));
    assert(space != NULL);
  }
  
  int length = space - start;
  char* temp = (char*)malloc(length + 1);
  strncpy(temp, start, length);
  temp[length] = '\0';
  *ptr = temp;

  return space + 1;
}

bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf)
{
  // 1读取请求行
  char* end = bufferFindCRLF(readBuf);
  // 2字符串起始地址
  char* start = readBuf->data + readBuf->readPos;
  // 请求行的长度
  int len = end - start;
  
  // get /xxx/xxx.txt http/1.1
  if(len)
  {
    // 请求方式
#if 0
    char* space = (char*)memmem(start, len, " ", 1);
    assert(space != NULL);

    int methonSize = space - start;
    request->methon = (char*)malloc(sizeof(char) * methonSize + 1);
    strncpy(request->methon, start, methonSize);
    request->methon[methonSize] = '\0';

    // 请求的资源
    start = space + 1;
    space = (char*)memmem(start, end - start, " ", 1);
    assert(space != NULL);

    int urlSize = space - start;
    request->url = (char*)malloc(sizeof(char) * urlSize + 1);
    strncpy(request->url, start, urlSize);
    request->methon[urlSize] = '\0';

    // 请求的版本
    start = space + 1;
    request->version = (char*)malloc(sizeof(char) * (end - start) + 1);
    strncpy(request->url, start, end - start);
    request->methon[end - start] = '\0';

    readBuf->readPos += len;
    readBuf->readPos += 2;
    request->curState = ParseReqHeaders;
    return true;
#else 
    start = spliteRequestLine(start, end, " ", &request->methon);
    start = spliteRequestLine(start, end, " ", &request->url);
    spliteRequestLine(start, end, NULL, &request->version);

#endif
  }

  return false;
}

bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf)
{
  char* end = bufferFindCRLF(readBuf);
  if(end != NULL)
  {
   char* start = readBuf->data + readBuf->readPos;  
   int lineSize = end - start;

   char* middle = (char*)memmem(start, lineSize, ": ", 2);
   if(middle != NULL)
   {
     char* key = (char*)malloc(middle - start + 1);
     strncpy(key, start, middle - start);
     key[middle - start] = '\0';

     char* value = (char*)malloc(end - middle - 2 + 1);
     strncpy(value, middle + 2, end - middle - 2);
     value[end - middle - 2] = '\0';

     httpRequestAddHeader(request, key, value);

     readBuf->readPos += lineSize;
     readBuf->readPos += 2;
   }
   else 
   {
      readBuf->readPos += 2;
      request->curState = ParseReqDone;
   }
    return true;
  }

  return true;
}


bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf, struct HttpRespone* respone, struct Buffer* sendBuf, int socket)
{
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
    case ParseReqBody:
      break;
    default:
      break;
    }

    if(!flag)
    {
      return flag;
    }

    // 判断是否解析完毕了，完毕了，需要准备回复的数据
    if(request->curState == ParseReqDone)
    {
      // 1.
      processHttpRequest(request,respone);
      // 2.
      httpResponsePrepareMsg(respone, sendBuf, socket);
    }
  }

  request->curState = ParseReqLine; // 状态还原，保证还能继续进行处理
  return  flag;
}

bool processHttpRequest(struct HttpRequest* request, struct HttpRespone* respone)
{
  // 目前只是处理get请求的。
  if(strcasecmp("get", request->methon) != 0)
  {
    return -1;
  }

  // 解码：中文英文的解码。
  decodeMsg(request->url, request->url);

  // 处理客户端请求的静态资源(目录或者文件)
  const char* file = NULL;
  if(strcmp(request->url, "/") == 0)
  {
    file = "./"; //相对路径，表示资源的根目录。
  }
  else 
  {
    file = request->url + 1; // 去掉/
  }
  
  struct stat st;
  int ret = stat(file, &st);

  if(ret == -1)
  {
    // 文件不存在的
    // sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
    // sendFile("404.html", cfd);
    strcpy(respone->fileName, "404.html");
    respone->statusCode = NotFound;
    strcpy(respone->statusMsg, "Not Found");
    // 响应头
    
    httpsResponeAddHeader(respone, "Content-type", getFileType(".html"));
    respone->sendDataFunc = sendFile;

    return true;
  }

  strcpy(respone->fileName, file);
  respone->statusCode = OK;
  strcpy(respone->statusMsg, "OK");
  // 判断文件类型
  if(S_ISDIR(st.st_mode))
  {
    // 把这个目录的内容发送客户端
    // sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
    // sendDir(file, cfd);
  
    httpsResponeAddHeader(respone, "Content-type", getFileType(".html"));
    respone->sendDataFunc = sendDir;

  }
  else 
  {
    // 把文件的内容发送给客户端
    // sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
    // sendFile(file, cfd);
    char tmp[12] = {0};
    sprintf(tmp, "%ld", st.st_size);
    httpsResponeAddHeader(respone, "Content-type", getFileType("file"));
    httpsResponeAddHeader(respone, "Content-length", getFileType(tmp));
    respone->sendDataFunc = sendFile;
  }

  request->curState = ParseReqLine;

  return true;
}

void decodeMsg(char* to, char* from)
{
  for (; *from != '\0'; ++to, ++from)
  {
    // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
    // Linux%E5%86%85%E6%A0%B8.jpg
    if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
    {
        // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
        // B2 == 178
        // 将3个字符, 变成了一个字符, 这个字符就是原始数据
        *to = hexToDec(from[1]) * 16 + hexToDec(from[2]);

        // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
        from += 2;
    }
    else
    {
        // 字符拷贝, 赋值
        *to = *from;
    }
  }
    *to = '\0';
}

int hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

const char* getFileType(const char* name)
{
  // a.jpg a.mp4 a.html
  // 自右向左查找‘.’字符，如不存在返回NULL
  const char* dot = strrchr(name, '.');
  if (dot == NULL)
    return "text/plain; charset=utf-8"; // 纯文本
  if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
    return "text/html; charset=utf-8";
  if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
    return "image/jpeg";
  if (strcmp(dot, ".gif") == 0)
    return "image/gif";
  if (strcmp(dot, ".png") == 0)
    return "image/png";
  if (strcmp(dot, ".css") == 0)
    return "text/css";
  if (strcmp(dot, ".md") == 0 || strcmp(dot, ".markdown") == 0)
    return "text/markdown; charset=utf-8";
  if (strcmp(dot, ".pdf") == 0)
    return "application/pdf";
  if (strcmp(dot, ".au") == 0)
    return "audio/basic";
  if (strcmp(dot, ".wav") == 0)
    return "audio/wav";
  if (strcmp(dot, ".avi") == 0)
    return "video/x-msvideo";
  if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
    return "video/quicktime";
  if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
    return "video/mpeg";
  if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
    return "model/vrml";
  if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
   return "audio/midi";
  if (strcmp(dot, ".mp3") == 0)
    return "audio/mpeg";
  if (strcmp(dot, ".ogg") == 0)
    return "application/ogg";
  if (strcmp(dot, ".pac") == 0)
    return "application/x-ns-proxy-autoconfig";

  return "text/plain; charset=utf-8";
}

void sendDir(const char* dirName,struct Buffer* sendBuf, int cfd)
{
    char buf[4096] = { 0 };
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName);
    struct dirent** namelist;
    int num = scandir(dirName, &namelist, NULL, alphasort);
    for (int i = 0; i < num; ++i)
    {
        // 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
        char* name = namelist[i]->d_name;
        struct stat st;
        char subPath[1024] = { 0 };
        sprintf(subPath, "%s/%s", dirName, name);
        stat(subPath, &st);
        if (S_ISDIR(st.st_mode))
        {
            // a标签 <a href="">name</a>
            sprintf(buf + strlen(buf), 
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", 
                name, name, st.st_size);
        }
        else
        {
            sprintf(buf + strlen(buf),
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        // send(cfd, buf, strlen(buf), 0);
        bufferAppendString(sendBuf, buf);
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    // send(cfd, buf, strlen(buf), 0);
    bufferAppendString(sendBuf, buf);
    free(namelist);
}

int sendFile(const char* fileName, struct Buffer* sendBuf, int cfd)
{
  // 打开文件，读一部分发一部分。
  int fd = open(fileName, O_RDONLY);
  assert(fd > 0);

#if 1
  while(1)
  {
    char buf[1024];
    int len = read(fd, buf, sizeof buf);
    if(len > 0)
    {
      // send(cfd, buf, len, 0);
      bufferAppendData(sendBuf, buf, len);
      usleep(10); // 这非常重要的。浏览器需要解析时间的。
    }
    else if(len == 0)
    {
      break;
    }
    else 
    {
      close(fd);
      perror("open");
      break;
    }
  }

#else 
  off_t offset = 0;
  int size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  while(offset < size)
  {
    sendfile(cfd, fd, &offset, size - offset);
  }
#endif
  close(fd);
  return 0;
}

