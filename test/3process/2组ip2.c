#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

int main()
{

  pid_t id = fork();
  assert(id >= 0);

  if(id == 0)
  {
    sleep(2);
    printf("子进程----\n");
    printf("PPID = %d\n", getppid()); // 当前bash的id值
    printf("PGRD = %d\n", getpgrp()); // 只有一个进程的时候，自己就是组长的
    printf("PID = %d\n", getpid());   // 这个程序加载到内存的id值
    while(1){;}
  }
  else 
  {
    printf("父进程---\n");
    printf("PPID = %d\n", getppid()); // 当前bash的id值
    printf("PGRD = %d\n", getpgrp()); // 只有一个进程的时候，自己就是组长的
    printf("PID = %d\n", getpid());   // 这个程序加载到内存的id值
    while(1){;}
  }

// bash
//   a.out
//     fork()

  return 0;
}
