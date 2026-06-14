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

        if(status & 0x7F)
        {
            int sig  = status & 0x7F;
            int core = (status>>7) & 1;
            printf("child killed by signal %d, core dump: %d\n", sig, core);
        }
        else 
        {
            int exit_code = (status>8) & 0XFF;
            printf("child exit code = %d\n", exit_code);
        }

    }

// wifexited
// wifsignaled

// wexitstatus
// wtermsig
    return 0;
}