#include <unistd.h>
#include <stdio.h>

int main()
{
    printf("process is running ....\n");
    // char* const agrs[] = {"ls", "-a", "-l", NULL};
    // char* const agrs[] = {"ls", "-a", "-l", "/" , NULL};
    char* const agrs[] = {"ls", "-a", "-l", "/tmp" , NULL};
    // char* const agrs[] = {"pwd", NULL};

    execv("/usr/bin/ls", agrs);
    
    printf("process is running ....\n");

    return 0;
}