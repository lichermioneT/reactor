#include<stdio.h>      
#include<unistd.h>      
#include<string.h>      
#include<stdlib.h>    

int add(int from, int to)
{
    int sum = 0;
    for(int i = from; i <= to; i++)
    {
        sum += i;
    }
    exit(3);
}

int main()
{
    printf("lichermionexx ");

    add(1, 100);
    exit(2);



    return 0;
}