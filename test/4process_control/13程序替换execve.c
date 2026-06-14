#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    // 参数列表（必须以 NULL 结束）
    char *argv[] = {
        "ls", 
        "-l", 
        "/",
        NULL
    };

    // 环境变量（也必须以 NULL 结束）
    char *envp[] = {
        "MY_NAME=LIC",
        "PATH=/usr/bin:/bin",
        NULL
    };

    printf("即将使用 execve 执行 /bin/ls ...\n");

    execve("/bin/ls", argv, envp);

    // execve 成功后不会返回
    perror("execve");
    return 1;
}
