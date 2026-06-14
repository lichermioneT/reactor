#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    printf("usage: ./a.out port dir\n");
    return -1;
  }
  
  uint16_t port = atoi(argv[1]);

  // 切换到指定路径下面
  chdir(argv[2]);

  // 初始化用于监听的套接字
  int lfd = initListenFd(port);

  // 启动服务器程序
  epollrun(lfd); 
  return 0;
}
