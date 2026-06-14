#include <unistd.h>
#include <stdio.h>

int main()
{
    printf("process is running ....\n");
    // execlp("ls", "ls", "-a", "-l", NULL);
    // execlp("ls", "ls", "-a", "-l", "/", NULL);
    // execlp("ls", "ls", "-a", "-l", "/tmp", NULL);
    // execlp("pwd", "pwd", NULL);
    // p就是环境变量，根据环境变量执行的
    
    printf("process is running ....\n");

    return 0;
}