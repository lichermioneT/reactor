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
  if(listenfd < 0)
  {
      perror("socket");
      return -1;
  }

  // 2. 设置端口复用
  int opt = 1;
  if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    perror("setsockopt");
    return -1;
  }

  // 3. 绑定 IP + 端口
  struct sockaddr_in local;
  memset(&local, 0, sizeof(local));

  local.sin_family = AF_INET;
  local.sin_port = htons(PORT);
  local.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(listenfd, (struct sockaddr*)&local, sizeof(local)) < 0)
  {
      perror("bind");
      close(listenfd);
      return -1;
  }

  // 4. 监听
  if(listen(listenfd, BACKLOG) < 0)
  {
      perror("listen");
      close(listenfd);
      return -1;
  }

// 1.select需要的额外的的数组进行
  int clientfd[FD_SETSIZE];
  for(int i = 0; i < FD_SETSIZE; ++i)
  {
    clientfd[i] = -1;
  }

// 2.监听文件描述符放到数组0位置,更新maxfd;
  clientfd[0] = listenfd;
  int maxfd = listenfd;

  while(true)
  {
    // 1.从额外的数组里面放到select需要的位图结构里面去的
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

    // 2.开始监听
    int n = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);

    if(n < 0)
    {
      perror("select");
      break;
    }

    // 2.1如果监听的文件描述符返回了，证明新的客户端来了，需要添加到额外的数组里面去的
    if(FD_ISSET(listenfd, &readfds))
    {
      struct sockaddr_in client;
      socklen_t len = sizeof(client);

      int connfd = accept(listenfd, (struct sockaddr*)&client, &len);
      if(connfd < 0)
      {
        perror("accept");
        continue;
      }

      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client.sin_addr, ip, INET_ADDRSTRLEN);

      cout << "new client connected" << endl;
      cout << "client ip: " << ip << endl;
      cout << "client port: " << ntohs(client.sin_port) << endl;
      cout << "connfd: " << connfd << endl;

      // 把新的通信 fd 加入 clientfds 数组
      int pos = -1;
      for (int i = 1; i < FD_SETSIZE; ++i)
      {
          if (clientfd[i] == -1)
          {
              pos = i;
              break;
          }
      }

      if (pos == -1)
      {
          cout << "too many clients" << endl;
          close(connfd);
      }
      else
      {
          clientfd[pos] = connfd;

          if (connfd > maxfd)
          {
              maxfd = connfd;
          }
      } 

      for (int i = 1; i < FD_SETSIZE; ++i)
      {
          int fd = clientfd[i];

          if (fd == -1)
          {
              continue;
          }

          if (FD_ISSET(fd, &readfds))
          {
              char buffer[BUFFER_SIZE];

              ssize_t ret = read(fd, buffer, sizeof(buffer) - 1);

              if (ret > 0)
              {
                  buffer[ret] = '\0';

                  cout << "client fd " << fd << " say: " << buffer << endl;

                  // 回显给客户端
                  write(fd, buffer, ret);
              }
              else if (ret == 0)
              {
                  cout << "client fd " << fd << " closed" << endl;

                  close(fd);
                  clientfd[i] = -1;
              }
              else
              {
                  perror("read");

                  close(fd);
                  clientfd[i] = -1;
              }
          }
        }
    }

  }


  return 0;
}
