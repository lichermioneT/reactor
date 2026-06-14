#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
using namespace std;

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    cout<< "usage: ./a.out port" <<endl;
    return -1;
  }
  uint16_t port = atoi(argv[1]);

// 1.udp服务器创建一个网络通信的文件描述符
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0)
  {
    perror("socket failed");
    return -1;
  }
  cout<< "1.sock success" <<endl;

  struct sockaddr_in local;
  bzero(&local, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = htonl(INADDR_ANY);
  local.sin_port = htons(port);

//2.将文件描述符和ip，port关联起来的
  if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0)
  {
    perror("bind failed");
    return -1;
  }
  cout<< "2.bind success" <<endl;

//3.开始监听服务启动
//这里只会存放128+1个三次握手成功的链接，如果生成不取走，就会抛弃的。
  if(listen(sock, 128) < 0)
  {
    perror("listen failed");
    return -1;
  }
  cout<< "3.listen success" <<endl;

  for (;;)
  {
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);

    // 4. 获取新的连接
    int sockfd = accept(sock, (struct sockaddr*)&peer, &len);
    if (sockfd < 0)
    {
        perror("accept failed");
        continue; // 不建议整个服务器直接退出
    }

    cout << "4.accept success" << endl;

    char inbuffer[1024] = {0};

    // 留一个位置给 '\0'
    ssize_t n = recv(sockfd, inbuffer, sizeof(inbuffer) - 1, 0);
    if (n > 0)
    {
        inbuffer[n] = '\0'; // 手动补字符串结束符
        cout << "接收到消息: " << inbuffer << endl;

        string message = "消息已经接收到了";
        ssize_t s = send(sockfd, message.c_str(), message.size(), 0);
        if (s < 0)
        {
            perror("send failed");
        }
    }
    else if (n == 0)
    {
        cout << "客户端已经关闭连接" << endl;
    }
    else
    {
        perror("recv failed");
    }

    close(sockfd); // 一次通信结束后关闭连接
  }
  return 0;
}
