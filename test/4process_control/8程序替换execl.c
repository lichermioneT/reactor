#include <unistd.h>
#include <stdio.h>

int main()
{
    printf("process is running ....\n");

    // execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
    // execl("/usr/bin/ls", "ls", "-a", "-l", "/",  NULL); 
    // execl("/usr/bin/ls", "ls", "-a", "-l", "/tmp",  NULL); 
    // execl("/usr/bin/pwd", "pwd", NULL);
    // execl("/usr/bin/cd", "cd", "..",  NULL); // 内建指令

    // l (list): 参数以独立参数的形式传递，每个参数都是一个字符串
    printf("process is running ....\n");

    return 0;
}