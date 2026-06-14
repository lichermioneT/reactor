#include "server.h"
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

int initListenFd(uint16_t port)
{
  // 1.创建监听的fd
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd <  0)
  {
    perror("socket");
    return -1;
  }

  // 2.设置端口复用
  int opt = 1;
  if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0)
  {
    perror("setsockopt");
    return -1;
  }
  
  // 3.绑定
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意网卡地址
  addr.sin_port = htons(port);

  if(bind(listenfd, (struct sockaddr*)&addr, sizeof addr) < 0)
  {
    perror("bind");
    return -1;
  }

  // 4.监听
  if(listen(listenfd, 128) < 0)
  {
    perror("listen");
    return -1;
  }

  return listenfd;
}

int epollrun(int lfd)
{
  // 1.创建epoll实例
  int epfd = epoll_create(1);
  if(epfd < 0)
  {
    perror("epfd");
    return -1;
  }

  // 2.lfd上树
  struct epoll_event ev; 
  ev.data.fd = lfd;
  ev.events = EPOLLIN;

  if(epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) < 0)
  {
    perror("epoll_ctl");
    return -1;
  }

  // 3.检查
  struct epoll_event evs[1024];
  size_t size = sizeof(evs) / sizeof(struct epoll_event);

  while(1)
  {
    int n = epoll_wait(epfd, evs, size, -1);
    for(int i = 0; i < n; ++i)
    {
      int fd  = evs[i].data.fd;
      if(fd == lfd)
      { 
        acceptClient(epfd, lfd);
        printf("新链接到来\n");
      }
      else 
      {
        // 通信的描述符
        recvHttpRequest(fd, epfd);
      }
    }
  }
  return 0;
}

int acceptClient(int epfd, int lfd)
{
  // 1.建立链接
  int cfd = accept(lfd, NULL, NULL);
  if(cfd  < 0)
  {
    perror("accept");
    return -1;
  }

  // 2.设置非阻塞
  int flag = fcntl(cfd, F_GETFL);
  flag |= O_NONBLOCK;
  fcntl(cfd, F_SETFL, flag);
  
  struct epoll_event ev;
  ev.data.fd = cfd;
  ev.events = EPOLLIN | EPOLLET;
  
  if(epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) < 0)
  {
    perror("epoll_ctl");
    return -1;
  }

  return 0;
}

int recvHttpRequest(int cfd, int epfd)
{
  int len = 0;
  int total = 0;
  char temp[1024] = {0};
  char buf[4096] = {0}; 
  while((len = recv(cfd, temp, sizeof temp, 0)) > 0)
  {
    if((size_t)total + len < sizeof buf)
    {
      memcpy(buf + total, temp, len);
    }
    else 
    {
      break;
    }

    total += len;
  }

  // 判断数据是否接收完毕
  if(len == -1 && errno == EAGAIN)
  {
    // 请求行解析
    char* pt = strstr(buf, "\r\n");
    if(pt == NULL)
    {
      epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
      close(cfd);
      return -1;
    }

    int reqlen = pt - buf;
    buf[reqlen] = '\0';
    parseRequestLine(buf, cfd);
  }
  else if(len == 0)
  {
    // 客户端断开链接了
    epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
    close(cfd);
  }
  else 
  {
    perror("recv");
  }
  return 0;
}

int parseRequestLine(const char* line, int cfd)
{
  // 请求行:get /xxx/1.jpg http/1.1 
  // /：资源的根目录
  char method[12] = {0}; 
  char path[1024] = {0};

  // 目前只是处理get请求的。
  sscanf(line,"%[^ ] %[^ ]", method, path);
  if(strcasecmp("get", method) != 0)
  {
    return -1;
  }

  decodeMsg(path, path);

  // 处理客户端请求的静态资源(目录或者文件)
  char* file = NULL;
  if(strcmp(path, "/") == 0)
  {
    file = "./"; //相对路径，表示资源的根目录。
  }
  else 
  {
    file = path + 1; // 去掉/
  }
  
  struct stat st;
  int ret = stat(file, &st);
  if(ret == -1)
  {
    // 文件不存在的
    sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
    sendFile("404.html", cfd);
    return -1;
  }

  // 判断文件类型
  if(S_ISDIR(st.st_mode))
  {
    // 把这个目录的内容发送客户端
    sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
    sendDir(file, cfd);
  }
  else 
  {
    // 把文件的内容发送给客户端
    sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
    sendFile(file, cfd);
  }

  return 0;
}

int sendFile(const char* fileName, int cfd)
{
  // 打开文件，读一部分发一部分。
  int fd = open(fileName, O_RDONLY);
  assert(fd > 0);

#if 0
  while(1)
  {
    char buf[1024];
    int len = read(fd, buf, sizeof buf);
    if(len > 0)
    {
      send(cfd, buf, len, 0);
      usleep(10); // 这非常重要的。浏览器需要解析时间的。
    }
    else if(len == 0)
    {
      break;
    }
    else 
    {
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

int sendHeadMsg(int cfd, int status, const char* descr, const char* type, int length)
{
  // 状态行
  char buf[4096] = {0};

#if 0
  // 响应头
  sprintf(buf,"http/1.1 %d %s\r\n", status, descr);
  sprintf(buf + strlen(buf), "content-type: %s\r\n", type);
  sprintf(buf + strlen(buf), "content-length: %d\r\n\r\n", length);

  send(cfd, buf, strlen(buf), 0);
#else 
  sprintf(buf,"HTTP/1.1 %d %s\r\n", status, descr);
  sprintf(buf + strlen(buf), "Content-Type: %s\r\n", type);
  sprintf(buf + strlen(buf), "Connection: close\r\n");
  if(length >= 0)
  {
    sprintf(buf + strlen(buf), "Content-Length: %d\r\n", length);
  }

  sprintf(buf + strlen(buf), "\r\n");
  send(cfd, buf, strlen(buf), 0);

#endif
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

int sendDir(const char* dirName, int cfd)
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
        send(cfd, buf, strlen(buf), 0);
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    send(cfd, buf, strlen(buf), 0);
    free(namelist);
    return 0;
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

