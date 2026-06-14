#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() 
{
    printf("祖先进程 PID: %d\n", getpid());
    pid_t son = fork();
    if (son == 0) 
    {
        printf("儿子出生 PID: %d, 父亲: %d\n", getpid(), getppid());
        pid_t grandson = fork();
        if (grandson == 0) 
        {
            printf("孙子出生 PID: %d, 父亲: %d\n", getpid(), getppid());
            while(1) sleep(1);
        }
        while(1) sleep(1);
    }
    while(1) sleep(1);
}