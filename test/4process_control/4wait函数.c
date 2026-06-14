#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{

    pid_t pid = fork();
    if(pid < 0)
    {
        perror("fork failed");
        exit(3);
    }
    
    if(pid == 0)
    {
        int cnt = 20;
        while(cnt)
        {
            printf("我是子进程, pid : %d ...\n", getpid());
            sleep(1);
            cnt--;
        }
    }
    else 
    {
        int status = 0;
        pid_t id = wait(&status);
        if(WIFEXITED(status))
        {
            printf("子进程退出， pid : %d, 退出码是 %d\n", id, WEXITSTATUS(status));
        }

        if(WIFSIGNALED(status))
        {
            printf("子进程信号退出， pid : %d, 退出信号是 %d\n", id, WTERMSIG(status)); // kill -number pid
        }

    }

// wifexited
// wifsignaled

// wexitstatus
// wtermsig
    return 0;
}