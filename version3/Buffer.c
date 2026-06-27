#define _GNU_SOURCE

#include "Buffer.h"
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct Buffer* bufferInit(int size)
{
  struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
  if(buffer != NULL)
  {
    buffer->data = (char*)malloc(size * sizeof(char));
    buffer->capacity = size;
    buffer->readPos = 0;
    buffer->writePos = 0;
    memset(buffer->data, 0, size);
  }

  return buffer;
}

void bufferDestory(struct Buffer* buffer)
{
  if(buffer != NULL)
  {
    if(buffer->data != NULL)
    {
      free(buffer->data);
    }
    free(buffer);
  }

  buffer = NULL;
}

void bufferExtendrRoom(struct Buffer* buffer, int size)
{
  // 1.内存够用,不需要扩容
  if(bufferWriateableSize(buffer) >= size)
  {
    return;
  }

  // 2.内存需要合并够用
  // 已经读了+剩余的 > size 
  else if(buffer->readPos + bufferWriateableSize(buffer) >= size)
  {
    // 到的没有读过内存的大小
    int readable = bufferReadableSize(buffer);
    // 移动内存
    memcpy(buffer->data, buffer->data + buffer->readPos, readable);
    // 更新位置
    buffer->readPos = 0;
    buffer->writePos = readable;
  }
  // 3.内存不够用.扩容
  else 
  {
    char* temp= (char*)realloc(buffer->data, buffer->capacity + size);
    if(temp == NULL)
    {
      return;
    }
    // 更新
    memset(temp + buffer->capacity, 0, size);
    buffer->data = temp;
    buffer->capacity = buffer->capacity + size;
  }
}

// 得到剩余可写的容量 
int bufferWriateableSize(struct Buffer* buffer)
{
  return  buffer->capacity - buffer->writePos;
}
// 得到剩余可写的容量
int bufferReadableSize(struct Buffer* buffer)
{
  return buffer->writePos - buffer->readPos;
}

int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
  if(buffer == NULL || data == NULL || size <= 0)
  {
    return -1;
  }
  
  bufferExtendrRoom(buffer, size);
  memcpy(buffer->data + buffer->writePos, data, size);
  buffer->writePos += size;

  return 0;
}

int bufferAppendString(struct Buffer* buffer, const char* data)
{
  int size = strlen(data);
  int ret = bufferAppendData(buffer, data, size);

  return ret;
}

int bufferSocketRead(struct Buffer* buffer, int fd)
{
  // read/recv/readv
  struct iovec vec[2];
  
  // 初始化数据
  int writeable = bufferWriateableSize(buffer);
  vec[0].iov_base = buffer->data + buffer->writePos;
  vec[0].iov_len = writeable;
  
  char* tmpbuf = (char*)malloc(40960);
  vec[1].iov_base = tmpbuf;
  vec[1].iov_len = 40960;
  
  int result = readv(fd, vec, 2);
  if(result == -1)
  {
    return -1;
  }
  else if(result < writeable)
  {
    buffer->writePos  += result;
  }
  else 
  {
    buffer->writePos = buffer->capacity;
    bufferAppendData(buffer, tmpbuf,result-writeable);
  }

  free(tmpbuf);

  return result;
}

char* bufferFindCRLF(struct Buffer* buffer)
{
  // strstr:大字符匹配小字符串
  // memmem:大数据块匹配小数据块
  char* ptr = (char*)memmem(buffer->data + buffer->readPos, bufferReadableSize(buffer), "\r\n", 2); 
  return ptr;
}

int bufferSendData(struct Buffer* buffer, int socket)
{
  // 判断数据有无
  int readable = bufferReadableSize(buffer);
  if(readable > 0)
  {
    int count = send(socket, buffer->data + buffer->readPos, readable, 0);
    if(count > 0)
    {
      buffer->readPos += count;
      usleep(1);
    }
    return  count;
  }

  return 0;
}
