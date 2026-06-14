#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <assert.h>

int main()
{
    pid_t pid = fork();
    assert(pid >= 0);

    if(pid == 0)
    {
        while(1)
        {
            printf("子进程S状态 \n"); // 等待外设和睡眠基本都是S状态，阻塞或者挂起状态。
            sleep(1);
        }
    }
    else 
    {
        int status = 0;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status))
        {
            printf("子进程正常退出，退出码%d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}