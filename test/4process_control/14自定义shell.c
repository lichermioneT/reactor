  #include <stdio.h>    
  #include <string.h>    
  #include <string.h>    
  #include <unistd.h>    
  #include <stdlib.h>    
  #include <sys/wait.h>    
  #include <sys/types.h>    
  #include <assert.h>    
      
  #define NUM 1024    
  #define OPT_NUM 64    
  char lineCommand[NUM];    
  char* myargv[OPT_NUM];    
      
      
int main(int argc, char* agrv[])    
{    
    while(1)  // 父进程一直循环，给子进程提供一个环境。
    {    
      
    // 获取用户输入    
    printf("用户名@主机名：当前路径# ");    
    fflush(stdout);    
      
    // 获取用户输入信息    
    char* s = fgets(lineCommand, sizeof(lineCommand) - 1, stdin);    
    assert(s != NULL);    
    (void)s;    
      
    // 清除最后一个\n    
    lineCommand[strlen(lineCommand) - 1] = 0;    
    /*printf("test : %s \n", lineCommand);*/    
      
      
    // 循环切割    
    myargv[0] = strtok(lineCommand, " ");    
    int i = 1;    
      
    while(myargv[i++] = strtok(NULL, " "));    
      
    /*    
     *for(int i = 0; myargv[i]; i++)    
     *{    
     *  printf("%d : %s \n", i, myargv[i]);     
     *}    
     */                                                                                                                                                                                                             
        
    // 执行指令了      
        
    pid_t id = fork();    
    assert(id != -1);    
    if(id == 0)
    {
      execvp(myargv[0], myargv);
      exit(1);
    }
  
    int status = 0;
    waitpid(id, &status, 0);
  
    }
  
  return 0;
}
