#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef void(*task)(); // 函数指针
void func1()
{
    printf("函数1....、\n");
}

void func2()
{
    printf("函数2....\n");
}

void func3()
{
    printf("函数3....\n");
}

task arr[3] = {func1, func2, func3};

int main()
{

    pid_t pid = fork();
    assert(pid >= 0);

    if(pid == 0)
    {
        int cnt = 8;
        while(cnt)
        {
            printf("我是子进程,我的pid : %d, cnt : %d\n", getpid(), cnt);
            sleep(1);
            cnt--;
        }
    }
    else 
    {
        int status = 0;
        pid_t ret = waitpid(pid, &status, WNOHANG); // 等待一次，没有成功就返回了，放在循环里面进行，每次循环的轮询检查
        while(1)
        {

            if(ret == 0)
            {
                printf("子进程还没有退出。。\n");
                sleep(2);
                arr[1]();
            }
            else if(ret > 0)
            {
                if(WIFEXITED(status))
                {
                    printf("子进程退出了，退出码是 %d\n", WEXITSTATUS(status));
                }

                if(WIFSIGNALED(status))
                {
                    printf("子进程退出了，退出信号是: %d\n", WTERMSIG(status));
                }
                break;
            }
            else 
            {
                printf("等待失败\n");
                break;
            }
        }


    }
    return 0;
}