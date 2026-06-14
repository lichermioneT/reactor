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
  int lastCode = 0;    
  int lastSig = 0;    
      
  int main()    
  {    
    while(1)    
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
    if(myargv[0] != NULL && strcmp(myargv[0], "ls") == 0)    
    {    
      myargv[i++] =(char*) "--color=auto";    
    }    

    while(myargv[i++] = strtok(NULL, " "));
  
  // cd命令，不需要创建子进程让shell自己执行对应的cd指令                                                                                                                     
  // 像这种不需要让我们子进程来执行，而是让shell自己执行的命令，内建内置命令
  // echo 
    if(myargv[0] != NULL && strcmp(myargv[0], "cd") == 0)
    {
      if(myargv[1] != NULL) chdir(myargv[1]);
      continue;
    }
    
    if(myargv[0] != NULL && myargv[1] != NULL && strcmp(myargv[0], "echo") == 0)
    {
      if(strcmp(myargv[1], "$?") == 0)
      {
        printf("%d, %d\n", lastCode, lastSig);
      }
      else 
      {
        printf("%s\n", myargv[1]);
      }
  
      continue;
    }
  
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
        pid_t ret = waitpid(id, &status, 0);
        assert(ret > 0);
        (void)ret;
        lastCode =  WIFSIGNALED(status);
        lastSig =  WTERMSIG(status);
    }

    return 0;
  }