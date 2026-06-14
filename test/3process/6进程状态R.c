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
        int a = 10;
        while(1)
        {
            a += 10;
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