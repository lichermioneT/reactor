#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "TcpServer.h"

int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    printf("usage: ./a.out port paht\n");
    return -1;
  }

  unsigned short port = atoi(argv[1]);
  chdir(argv[2]);

  struct TcpServer* server = tcpServerInit(port, 4);
  TcpServerRun(server);

  return 0;
}
