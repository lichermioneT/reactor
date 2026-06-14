#include "udpclient.hpp"
#include <cstring>

udpclient::udpclient(const string& ip, uint16_t port)
  :_ip(ip)
  ,_port(port)
  ,_sock(-1)
{}

udpclient::~udpclient()
{
  if(_sock >= 0)
  {
    close(_sock);
    _sock = -1;
  }
}

bool udpclient::createSocket()
{

  _sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(_sock < 0)
  {
    perror("sock failed");
    return false;
  }

  return true;
}

bool udpclient::buildServerAddr(struct sockaddr_in* server)
{
  memset(server, 0, sizeof(*server));
  server->sin_family = AF_INET;
  server->sin_port = htons(_port);

  int ret = inet_pton(AF_INET, _ip.c_str(), &(server->sin_addr));
  if(ret != 1)
  {
    if(ret == 0)
    {
      cerr<< "invaild ip address:" << _ip <<endl;
    }
    else 
    {
      perror("inet_pton failed");
    }
    return false;
  }
  return true;
}


bool udpclient::init()
{
  if(!createSocket())
  {
    return false;
  }

  return true;
}

void udpclient::run()
{
  struct sockaddr_in server;
  if(!buildServerAddr(&server))
  {
    return;
  }

  for(;;)
  {
    string message;
    cout<< "请输入";
    getline(cin, message);

    if(!cin)
    {
      cout<< "输入结束，客户端退出"  << endl;
    }

    if (message == "quit" || message == "exit")
    {
         cout << "客户端主动退出" << endl;
         break;
     }
    ssize_t sendSize = sendto(
            _sock,
            message.c_str(),
            message.size(),
            0,
            (struct sockaddr*)&server,
            sizeof(server)
        );

        if (sendSize < 0)
        {
            perror("sendto failed");
            continue;
        }

        char inbuffer[1024] = {0};
        ssize_t recvSize = recvfrom(
            _sock,
            inbuffer,
            sizeof(inbuffer) - 1,
            0,
            nullptr,
            nullptr
        );

        if (recvSize < 0)
        {
            perror("recvfrom failed");
            continue;
        }

        inbuffer[recvSize] = '\0';
        cout << "server echo# " << inbuffer << endl;
    }
}

void udpclient::start()
{
    if (_sock < 0)
    {
        cerr << "socket is not ready, please call init() first" << endl;
        return;
    }

    run();
}
