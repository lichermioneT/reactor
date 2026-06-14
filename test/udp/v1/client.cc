#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;


int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    cout<< "usage: ./a.out ip port" << endl; 
    return -1;
  }
  
  string ip = argv[1];
  uint16_t port = atoi(argv[2]);

// 1.创建一个网络通信的文件描述符
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0)
  {
    perror("socket failed");
    return -1;
  }
  cout<<  "1.sock success" << endl;

// 客户端不需要显示的bind, OS会自动给你bind的。
// 需要设置进内核，建议清零一下数据的
  struct sockaddr_in server;
  bzero(&server, sizeof(server));
// 通信协议，ip，port三件套
  server.sin_family = AF_INET;
  if(inet_pton(AF_INET, ip.c_str(), &server.sin_addr) < 0)
  {
    perror("inet_pton failed");
    return -1;
  }
  server.sin_port = htons(port);

  for(;;)
  {
    char buffer[1024] = {0};
    char inbuffer[1024] = {0};
    cout<< "请输入:";
    cin.getline(buffer, 1024);
// sendto需要知道，发送端的信息，后面两个参数进行填充的
    sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&server, sizeof(server));
    cout<<  "2.sendto success" << endl;

// recvfrom 谁给发送的你可以知道。输出型参数的。
    recvfrom(sock, inbuffer, sizeof(inbuffer) - 1, 0, nullptr, 0);
    cout<< "recvfrom success:" << inbuffer <<endl;
  }
}
