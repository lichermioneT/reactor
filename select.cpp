#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080
#define BACKLOG 128
#define BUFFER_SIZE 1024


int main()
{

// 1. 创建监听 socket
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0)
  {
      perror("socket");
      return -1;
  }

  // 2. 设置端口复用
  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // 3. 绑定 IP + 端口
  struct sockaddr_in local;
  memset(&local, 0, sizeof(local));

  local.sin_family = AF_INET;
  local.sin_port = htons(PORT);
  local.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listenfd, (struct sockaddr*)&local, sizeof(local)) < 0)
  {
      perror("bind");
      close(listenfd);
      return -1;
  }

  // 4. 监听
  if (listen(listenfd, BACKLOG) < 0)
  {
      perror("listen");
      close(listenfd);
      return -1;
  }

// 额外的的数组进行
  int clientfd[FD_SETSIZE];
  for(int i = 0; i < FD_SETSIZE; ++i)
  {
    clientfd[i] = -1;
  }

  clientfd[0] = listenfd;

  int maxfd = listenfd;

  while(true)
  {
    fd_set readfds;
    FD_ZERO(&readfds);
    
    for(int i = 0; i < FD_SETSIZE; ++i)
    {
      if(clientfd[i] != -1)
      {
        FD_SET(clientfd[i], &readfds);

        if(clientfd[i] > maxfd)
        {
          maxfd = clientfd[i];
        }
      }
    }

    int n = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);

    if(n < 0)
    {
      perror("select");
      break;
    }

    if(FD_ISSET(listenfd, &readfds))
    {

    }


  }




  return 0;
}
