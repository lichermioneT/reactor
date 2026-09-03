#pragma once

// 读http的数据存储下来

struct Buffer
{
// 指向内存的指针
  char* data;
// 容量
  int capacity;
// 读数据位置
  int readPos;
// 写数据位置
  int writePos;
};


// 初始化
struct Buffer* bufferInit(int size);
// 销毁
void bufferDestory(struct Buffer* buffer);
// 扩容
void bufferExtendRoom(struct Buffer* buffer, int size);
// 剩余可写容量
int bufferWriteableSize(struct Buffer* buffer);
// 剩余可读容量
int bufferReadableSize(struct Buffer* buffer);
// 直接写
int bufferAppendData(struct Buffer* buffer, const char* data, int size);
int bufferAppendString(struct Buffer* buffer, const char* data);
// 接收套接字
int bufferSocketRead(struct Buffer* buffer, int fd);
// 根据\r\n取出一行
char* bufferFindCRLF(struct Buffer* buffer);
