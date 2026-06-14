#include <stdio.h>
#include <stdlib.h>

// 代码段（text segment）中的函数
void func()
{
    printf("Inside func\n");
}

// 全局已初始化变量 —— data segment
int global_init = 100;

// 全局未初始化变量 —— bss segment
int global_uninit;

// 字符串常量 —— rodata
const char* str = "Hello address space";

// 静态区（已初始化）—— data segment
static int static_init = 200;

// 静态区（未初始化）—— bss segment
static int static_uninit;

int main()
{
    // 栈区
    int stack_var = 10;

    // 堆区
    int* heap_var = (int*)malloc(sizeof(int));
    *heap_var = 20;

    printf("========= 进程地址空间演示（含静态区） =========\n\n");

    // 代码段
    printf("代码段（text）函数地址:          %p\n", func);

    // rodata
    printf("只读数据区（rodata）地址:        %p\n", str);

    // global data
    printf("全局变量 已初始化（data）地址:   %p\n", &global_init);
    printf("全局变量 未初始化（bss）地址:    %p\n", &global_uninit);

    // static data
    printf("静态变量 已初始化（data）地址:   %p\n", &static_init);
    printf("静态变量 未初始化（bss）地址:    %p\n", &static_uninit);

    // heap
    printf("堆区（heap）地址:               %p\n", heap_var);

    // stack
    printf("栈区（stack）地址:              %p\n", &stack_var);

    printf("\n内存空间一般布局（从低到高）：\n");
    printf("[text] -> [rodata] -> [data/static_init] -> [bss/static_uninit] -> [heap] -> ... -> [stack]\n");

    while(1); // 保持进程，方便 pmap 查看
    return 0;
}
