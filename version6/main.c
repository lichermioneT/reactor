#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    printf("usage: ./a.out port dir\n");
    return -1;
  }
  
  unsigned short  port = atoi(argv[1]);

  // 切换到指定路径下面
  chdir(argv[2]);


  return 0;
}
