#include "HttpRequest.h"
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>

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
  if(request->reHeaders)
  {
    for(int i = 0; i <  request->reqHeadersNum; ++i)
    {
      free(request->reHeaders[i].key);
      free(request->reHeaders[i].value);
    }
    free(request->reHeaders);
  }
  httpRequestReset(request);
}

void httpRequestDestory(struct HttpRequest* request)
{
  if(request)
  {
    httpRequestResetEx(request);
    free(request);
  }
}

enum HttpRequestState httpRequestState(struct HttpRequest* request)
{
  return  request->curState;
}

void httRequestAddHeader(struct HttpRequest* request, char* key, char* values)
{
  request->reHeaders[request->reqHeadersNum].key = key;
  request->reHeaders[request->reqHeadersNum].value = values;
  request->reqHeadersNum++;
}

char* httRequestGetHeader(struct HttpRequest* request, char* key)
{
  if(request)
  {
    for(int i = 0; i < request->reqHeadersNum; ++i)
    {
      //if(strcmp(key, request->reHeaders[i].key) == 0)
      if(strncasecmp(key, request->reHeaders[i].key, strlen(key))  == 0)
      {
        return request->reHeaders[i].value;
      }
    }
  }

  return NULL;
}

char* splitRequestLine(const char* start, const char* sub, char** ptr)
{
  char* space = NULL;
  if(sub)
  {
    space = strstr(start, sub);
  }

  int length = space - start;
  char* temp = (char*)malloc(length + 1);
  strncpy(temp, start, length);
  temp[length] = '\0';
  *ptr = temp;

  return space + 1;
}

bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* buffer)
{
  // 读出请求行
  // 结束地址
  char* end  = bufferFindCRLF(buffer);
  // 起始地址
  char* start = buffer->data + buffer->readPos; 
  // 请求行总长度
  int lineSize = end - start;
  
  if(lineSize)
  {
    // 请求的方式
    // get /xx/xx http/1.1
    char* space = strstr(start, " ");
    assert(space != NULL);

    int methonSize = space - start;
    
    request->methon = (char*)malloc(methonSize + 1);
    strncpy(request->methon, start, methonSize);
    request->methon[methonSize] = '\0';
    
    start = space + 1;
    space = strstr(start, " ");
    int urlSize = space - start;
    request->url = (char*)malloc(urlSize + 1);
    strncpy(request->url, start, urlSize);
    request->url[urlSize] = '\0';

    start = space + 1;
    int versionSize = end - start;
    request->version = (char*)malloc(end - start + 1);
    strncpy(request->version, space, end - start);
    request->version[versionSize] = '\0';
   
    // 后移
    buffer->readPos = lineSize + 2;
    // 状态
    request->curState = ParseReqHeaders;

    return true;
  }

  // 保存字符串起始地址
  // 保存字符串结束地址
  
  return false;
}

bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* buffer)
{
  char* end = bufferFindCRLF(buffer);
  if(end)
  {
    char* start = buffer->data + buffer->readPos;
    int lineSize = end - start;
    char* middle = strstr(start, ": ");
    if(middle)
    {
      char* key = (char*)malloc(middle - start + 1);
      strncpy(key, start, middle - start);
      key[middle-start] = '\0';

      char* value = (char*)malloc(end - middle - 2 + 1);
      strncpy(value, middle + 2, end - middle - 2);
      value[end - middle - 2] = '\0';
      
      httRequestAddHeader(request, key, value);

      buffer->readPos += lineSize;  
      buffer->readPos += 2;
    }
    else 
    {
      // 请求头被解析完了，跳过空行
      buffer->readPos += 2;
      // 修改请求状态
      request->curState = ParseReqDone;
    }

    return true;
  }

  return false;
}

bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* buffer, struct HttpRespone* respone, struct Buffer* sendbuf, int socket)
{
  bool flag = true;
  while(request->curState != ParseReqDone)
  {
    switch(request->curState)
    {
      case ParseReqLine:
        flag = parseHttpRequestLine(request, buffer);
        break;
      case ParseReqHeaders:
        flag = parseHttpRequestHeader(request, buffer);
        break;
      case ParseReqBody:
        break;
      default:
        break;
    }

    if(!flag)
    {
      return flag;
    }
    
    // 是否解析完了，准备需要回复的数据
    if(request->curState == ParseReqDone)
    {
      // 1.根据解析数，做处理
      processHttpRequest(request, respone);
      // 2.组织协议
      httpResponePrepareMsg(respone, sendbuf, socket);
    }

  }

  request->curState  = ParseReqLine;
  return flag;
}

// 处理get
bool processHttpRequest(struct HttpRequest* request, struct HttpRespone* respone)
{
  if(strcasecmp(request->methon, "get") != 0)
  {
    return -1;
  }
  
  decodeMsg(request->url, request->url);


  const char* file = NULL;
  if(strcmp(request->url, "/") == 0)
  {
      file = "./";
  }
  else 
  {
    file = request->url + 1;
  }

  struct stat st;
  int ret = stat(file, &st);
  if(ret == -1)
  {
     strcpy(respone->fileName, "404.html");
     respone->statusCode = NotFound;
     strcpy(respone->statusMsg, "Not Found");
    
     httpResponeAddHeader(respone, "Content-type", getFileType(".html"));
     respone->sendDataFunc = sendFile;
     return 0;
  }
  
   strcpy(respone->fileName, file); 
   respone->statusCode = OK;
   strcpy(respone->statusMsg, "OK");
  if(S_ISDIR(st.st_mode))
  {
     httpResponeAddHeader(respone, "Content-type", getFileType(".html"));
     respone->sendDataFunc = sendDir;
  }
  else 
  {
     char tmp[128] = {0};
     sprintf(tmp, "%ld", st.st_size);
     httpResponeAddHeader(respone, "Content-type", getFileType(file));
     httpResponeAddHeader(respone, "Content-length", tmp);
     respone->sendDataFunc = sendFile;
  }

  return false;
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

// 解码
// to 存储解码之后的数据, 传出参数, from被解码的数据, 传入参数
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


int sendFile(const char* fileName,struct Buffer* buffer, int cfd)
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
      bufferAppendData(buffer, buf, len);
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
    int ret = sendfile(cfd, fd, &offset, size);
    printf("ret:%d\n", ret);
    if(ret == -1)
    {
      perror("sendfile");
    }
  }
#endif
  close(fd);
  return 0;
}

void sendDir(const char* dirName, struct Buffer* buffer, int cfd)
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
        bufferAppendString(buffer, buf);
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    // send(cfd, buf, strlen(buf), 0);
    bufferAppendString(buffer, buf);
    free(namelist);
}
