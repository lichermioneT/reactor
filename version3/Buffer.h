#pragma once

struct Buffer
{
  char* data;
  int capacity;
  int readPos;
  int writePos;
};

//1.初始化buffer
struct Buffer* bufferInit(int size);
// 2.释放内存的函数
void bufferDestory(struct Buffer* buffer);
// 3.扩容
void bufferExtendrRoom(struct Buffer* buffer, int size);
// 得到剩余可写的容量 
int bufferWriateableSize(struct Buffer* buffer);
// 得到剩余可写的容量
int bufferReadableSize(struct Buffer* buffer);
// 4.写内存的函数
int bufferAppendData(struct Buffer* buffer, const char* data, int size);
int bufferAppendString(struct Buffer* buffer, const char* data);
int bufferSocketRead(struct Buffer* buffer, int fd);
// 5.根据\r\n
char* bufferFindCRLF(struct Buffer* buffer);

int bufferSendData(struct Buffer* buffer, int socket);
