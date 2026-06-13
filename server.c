#include "server.h"
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
      }
      else 
      {
        // 通信的描述符
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
