#include <stdio.h>
#include <unistd.h>

extern char** environ;

void print_all_env()
{
    for(int i = 0; *(environ+i); i++)
    {
        printf("%s \n", environ[i]);
    }
}

int main()
{

    print_all_env();






    return 0;
}