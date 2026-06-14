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
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
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

//3.服务器是一个死循环，常驻内存进程。
  for(;;)
  {
    char buffer[1024] = {0};
    struct sockaddr_in peer;    
    socklen_t len = sizeof(peer);
    char ip_str[INET_ADDRSTRLEN];

//4.上层没有设置，这就是阻塞等待。
    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&peer, &len);
    if(n > 0)
    {
      buffer[n] = 0;

// 5.服务器使用ntop,用来存储ip地址的。
// 宏INET_ADDRSTRLEN
      if(inet_ntop(AF_INET, &peer.sin_addr, ip_str, INET_ADDRSTRLEN) == nullptr)
      {
        perror("inet_ntop failed");
        return -1;
      }
      cout<< "3.inet_ntop success" <<endl;
      cout<< "客户端ip:" << ip_str << ",客户端口:" << ntohs(peer.sin_port) << ", message:" << buffer <<endl;
    }
   
// 6.udpsendto需要知道对方的地址信息呢。
    string message = "客户端您的消息已经收到了";
    sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr*)&peer, sizeof(peer));
  }
  close(sock);
  return 0;
}
