#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{

  printf("PPID = %d\n", getppid()); // 当前bash的id值
  printf("PID = %d\n", getpid());   // 这个程序加载到内存的id值
  printf("PGID = %d\n", getpgrp()); // 只有一个进程的时候，自己就是组长的

// bash
// a.out grp

  return 0;
}
