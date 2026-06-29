#include "TcpServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    printf("usage: ./server port path\n");
    return -1;
  }

  unsigned short port = (unsigned short)atoi(argv[1]);
  if(chdir(argv[2]) != 0)
  {
    perror("chdir");
    return -1;
  }

  // MODIFIED: check server initialization before entering the reactor loop.
// 1.初始化一个server对象
  struct TcpServer* server = tcpServerInit(port, 4);
  if(server == NULL)
  {
    return -1;
  }
// 2.运行server对象的
  TcpServerRun(server);
  return 0;
}
