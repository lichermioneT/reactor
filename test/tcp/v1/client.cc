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
  
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0)
  {
    cout<<  "sock failed" << endl;
    return -1;
  }
  cout<<  "1.sock failed" << endl;
 
  struct sockaddr_in server;
  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  if(inet_pton(AF_INET, ip.c_str(), &server.sin_addr) < 0)
  {
    cout<< "inet_pton failend" <<endl;
    return -1;
  }
  server.sin_port = htons(port);

  int n = connect(sock, (struct sockaddr*)&server, sizeof(server));
  if(n == -1)
  {
    perror("connect failed");
    return -1;
  }

  for(;;)
  {
    char buffer[1024] = {0};
    char inbuffer[1024] = {0};
    cout<< "请输入:";
    cin.getline(buffer, 1024);
   
    send(sock, buffer,sizeof(buffer), 0);
    cout<<  "2.send success" << endl;
    
    recv(sock, inbuffer, sizeof(inbuffer), 0);
    cout<< "接收到消息" << inbuffer <<endl;
  }
}
