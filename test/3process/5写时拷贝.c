#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int g_val  = 10;

int main()
{
    pid_t id = fork();
    if(id < 0)
    {
        perror("fork failed:");
    }
    else if(id == 0)
    {
        g_val = 0;
        int cnt = 0;
        while(cnt < 10)
        {
            printf("子进程g_val : %d, &g_val %p\n", g_val, &g_val);
            sleep(1);
            cnt++;
        }
    }
    else 
    {
        g_val = 1;
        int cnt = 0;
        while(cnt < 10)
        {
            printf("父进程g_val : %d, &g_val %p\n ", g_val, &g_val);
            sleep(1);
            cnt++;
        }

        int status = 0;
        pid_t ret = waitpid(id, &status, 0);

        if(WIFEXITED(status))
        {
            printf("子进程正常退出，退出码 %d\n", WEXITSTATUS(status));
        }
    }
  return 0;
}
