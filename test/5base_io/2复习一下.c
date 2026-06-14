#include <stdio.h>
#include <string.h>

int main()
{
    FILE* pf = fopen("./text.txt","r");
    if(pf == NULL)
    {
        perror("fopen failed");
    }
    
    char buffer[64] = {0};
    while(fgets(buffer, sizeof(buffer) - 1, pf) != NULL)
    {
        buffer[strlen(buffer) - 1] = 0;
        puts(buffer);
    }
    fclose(pf);
    pf = NULL;


    return 0;
}