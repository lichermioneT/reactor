#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int  main()
{
    umask(0);
    int fd = open("./text.txt", O_RDONLY, 0666);
    if(fd < 0)
    {
        perror("open failed");
        exit(1);
    }
    else 
    {
        printf("file discriptor : %d \n", fd);
    }

    char buffer[1024] = {0};
    ssize_t s = read(fd, buffer, sizeof(buffer) - 1); // read data 
    if(s > 0) buffer[s] = 0;
    printf("%s \n", buffer);

    close(fd);

    return 0;
}