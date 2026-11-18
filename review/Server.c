#include "Server.h"
#include <stdio.h>
#include <arpa/inet.h>

int initListenFd(unsigned short port)
{
// 1.创建一个网络通信的描述符
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  if(lfd == -1)
  {
    perror("socket");
    return -1;
  }

// 2.设置端口复用
  int opt = 1;
  int ret  = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
  if(ret == -1)
  {
    perror("setsockopt");
    return -1;
  }
  
  struct sockaddr_in address;
  address.sin_family  = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr= htonl(INADDR_ANY);

// 3.绑定 ip+port
  ret = bind(lfd, (struct sockaddr*)&address, sizeof address);
  if(ret == -1)
  {
    perror("bind");
    return -1;
  }

// 4.开始监听
  ret = listen(lfd, 128);
  if(ret == -1)
  {
    perror("listen");
    return -1;
  }

// 5.网络文件描述符给调用者
  return lfd;
}
