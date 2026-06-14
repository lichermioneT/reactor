#include <stdio.h>

int main()
{

    FILE* pf = fopen("./text.txt", "w");
    if(pf == NULL)
    {
        perror("fopen failed");
        return 1;
    }

    int cnt = 5;
    while(cnt)
    {
        fprintf(pf, "lichermionex %d \n", cnt--);
    }
    fclose(pf);
    pf = NULL;



    return 0;
}