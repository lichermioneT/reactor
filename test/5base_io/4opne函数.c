#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    umask(0);
    // int fd = open("./zz", O_RDONLY,0666);
    // int fd = open("./zz", O_WRONLY,0666);
    // int fd = open("./zz", O_CREAT,0666);
    // int fd = open("./zz", O_CREAT | O_EXCL,0666);
    // int fd = open("./zz", O_WRONLY | O_APPEND,0666);
    int fd = open("./zz", O_WRONLY | O_TRUNC,0666);



    if(fd < 0)
    {
        perror("open failed");
    }
    else 
    {
        printf("%d \n", fd);
    }

    close(fd);
    return 0;
}