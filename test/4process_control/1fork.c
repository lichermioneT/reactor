#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

int g_val = 100;

int main()
{
    printf("开始 pid : %d, g_val:%d, &g_val:%p\n", getpid(), g_val, &g_val);

    pid_t id = fork();
    assert(id >= 0);
    if(id == 0)
    {
        sleep(2);
        g_val = 999;
        printf("我是子进程pid:%d, ppid:%d, g_val=%d, &g_val:%p \n", getpid(), getppid(), g_val, &g_val);
    }
    else 
    {
        sleep(1);
        g_val = 888;
        printf("我是父进程pid:%d, ppid:%d, g_val=%d, &g_val:%p \n", getpid(), getppid(), g_val, &g_val);

        int status = 0;
        waitpid(id, &status, 0);
        if(WIFEXITED(status))
        {
            printf("子进程正常 %d\n", WEXITSTATUS(status));
        }
        
    }


/*
虚拟地址 VA = [PML4] [PDPT] [PD] [PT] [Offset]
                    |      |     |     |
                    v      v     v     v
                 查页表层级结构（从 CR3 开始）
                           |
                           v
                得到物理页框号 PFN
                           |
                           v
          物理地址 PA = PFN << 12 | Offset


*/








    return 0;
}