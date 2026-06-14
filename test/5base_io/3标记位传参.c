#include <stdio.h>

#define ONE   (1<<0)
#define TWO   (1<<1)
#define THREE (1<<2)
#define FOUR  (1<<3)

void show(int flags)
{
    if(flags & ONE) printf("ONE \n");      // 按位与
    if(flags & TWO) printf("TWO \n");
    if(flags & THREE) printf("THRER \n");
    if(flags & FOUR)  printf("FOUR \n");
}


int main()
{
    show(ONE | TWO);
    show(ONE | TWO | THREE | FOUR);


    return 0;
}