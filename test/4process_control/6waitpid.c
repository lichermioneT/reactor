#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{

    pid_t pid = fork();
    assert(pid >= 0);

    if(pid == 0)
    {
        int cnt = 15;
        while(cnt)
        {
            printf("我是子进程,我的pid : %d, cnt : %d\n", getpid(), cnt);
            sleep(1);
            cnt--;
        }
    }
    else 
    {
        int status = 0;
        pid_t ret = waitpid(pid, &status, 0); // 阻塞等待 等价wait函数
        if(WIFEXITED(status))
        {
            printf("子进程退出了，退出码是 %d\n", WEXITSTATUS(status));
        }

        if(WIFSIGNALED(status))
        {
            printf("子进程退出了，退出信号是: %d\n", WTERMSIG(status));
        }
    }
    return 0;
}