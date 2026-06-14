#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main()
{
    umask(0);
    int fd = open("./text.txt", O_WRONLY | O_APPEND, 0666);
    if(fd < 0)
    {
        perror("open failed");
        exit(1);
    }

    char buffer[1024] = "lichermionex\n";
    int cnt = 5;
    while(cnt)
    {
        write(fd, buffer, sizeof(buffer));
        cnt--;
    }
    close(fd);
    return 0;
}