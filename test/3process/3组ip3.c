#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

int main()
{
    pid_t arr[10] = {0};

    for(int i = 0; i < 10; i++)
    {
        arr[i] = fork();
        assert(arr[i] >= 0);

        if(arr[i] == 0) // 子进程
        {
            printf("子进程 %d: PID=%d, PPID=%d, PGRP=%d\n",
                   i, getpid(), getppid(), getpgrp());
            while(1){} // 保持子进程运行，方便观察
        }
        else // 父进程
        {
            printf("父进程创建第 %d 个子进程: 子PID=%d, 父PID=%d\n",
                   i, arr[i], getpid());
        }
    }


    // 父进程也保持运行
    while(1){}

// bash 
//   a.out 组长
//   group[10] 成员

/*
          ┌─────────────┐
          │   bash      │  <-- 父 shell
          │  PID=3341   │
          └─────┬───────┘
                │ 启动 a.out
          ┌─────▼───────┐
          │  a.out      │  <-- 父进程，组长
          │  PID=3393   │
          │  PGID=3393  │
          └─────┬───────┘
        ┌───────┴───────────────┐
        │       fork 10 次      │
┌───────┴───────┐ ┌───────┴───────┐     ┌───────┴───────┐
│ 子进程1       │ │ 子进程2        │     │ 子进程10       │
│ PID=3394      │ │ PID=3395      │     │ PID=3403      │
│ PGID=3393     │ │ PGID=3393     │     │ PGID=3393     │
└───────────────┘ └───────────────┘     └───────────────┘
*/

    return 0;
}
