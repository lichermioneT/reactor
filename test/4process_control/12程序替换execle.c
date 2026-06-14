#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t id = fork();
    if(id < 0)
    {
        perror("fork");
        return 1;
    }

    if(id == 0)
    {
        // 自定义环境变量
        char *new_env[] = {
            "NAME=LIC",
            "AGE=20",
            "PATH=/usr/bin:/bin", // 仍需要 PATH
            NULL
        };

        // arg0 为程序名，最后一个 NULL 结尾
        execle("/usr/bin/env", "env", NULL, new_env);

        // execle 失败才会执行这里
        perror("execle");
        exit(1);
    }
    else
    {
        printf("父进程等待子进程...\n");
        wait(NULL);
    }

// ✔ 1. 执行另一个程序，并且
// ✔ 2. 不继承当前进程的环境变量，而是使用你指定的新环境变量

    return 0;
}
