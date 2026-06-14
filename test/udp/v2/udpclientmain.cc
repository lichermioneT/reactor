#include "udpclient.hpp"
#include <memory>


// ./a.out ip port
int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    cout<<"usage: ./a.out ip port" <<endl;
    return -1;
  }

  string ip = argv[1];
  uint16_t port = atoi(argv[2]);
  
  udpclient cli(ip, port);
  if(!cli.init())
  {
    return 1;
  }

  cli.start();
  return 0;
}


