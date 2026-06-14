#include <unistd.h>
#include <stdio.h>

int main()
{
    printf("process is running ....\n");
    // char* const argv[] = {"ls", "-a", "-l", "-h", NULL};
    char* const argv[] = {"ls", "-a", "-l", "-h", "/", NULL};
    execvp("ls", argv);
    printf("process is running ....\n");
    
    return 0;
}