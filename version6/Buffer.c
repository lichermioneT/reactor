#include "Buffer.h"
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

struct Buffer* bufferInit(int size)
{
  struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
  if(buffer)
  {
      buffer->data = (char*)malloc(sizeof(char) * size);
      memset(buffer->data, 0, size);
      buffer->capacity = size;
      buffer->writePos = buffer->readPos = 0;
  }

  return buffer;
}

void bufferDestory(struct Buffer* buffer)
{
  if(buffer)
  {
    if(buffer->data)
    {
      free(buffer->data); 
    }
    free(buffer);
  }
}

// 剩余可写容量
int bufferWriteableSize(struct Buffer* buffer)
{
  return buffer->capacity - buffer->writePos;
}
// 剩余可读容量
int bufferReadableSize(struct Buffer* buffer)
{
  return buffer->writePos - buffer->readPos;
}

void bufferExtendRoom(struct Buffer* buffer, int size)
{
  // 1.内存够用---不需要扩容
  if(bufferWriteableSize(buffer) > size)
  {
    return;
  }
  // 2.内存够用，需要合合并
  else if(buffer->readPos + bufferWriteableSize(buffer) >= size)
  {
    // 未读内存的大小
    int readable = bufferReadableSize(buffer);
    // 内存拷贝
    memcpy(buffer->data, buffer->data + buffer->readPos, readable);
    // 更新位置
    buffer->readPos = 0;
    buffer->writePos = readable;

  }
  // 3.内存不够用，扩容
  else 
  {
    char* temp = (char*)realloc(buffer->data, buffer->capacity + size);
    if(temp == NULL)
    {
      return;
    }
    
    memset(temp + buffer->capacity, 0, size);
    buffer->data = temp;
    buffer->capacity += size;
  }
}

int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
  if(buffer == NULL || data == NULL || size <= 0)
  {
    return -1;
  }
  
  // 扩容
  bufferExtendRoom(buffer, size);

  // 数据拷贝
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
  // 初始化
  int writeable = bufferWriteableSize(buffer);
  vec[0].iov_base = buffer->data + buffer->writePos;
  vec[0].iov_len = writeable;

  char* temp = (char*)malloc(40960);
  vec[1].iov_base = temp;
  vec[1].iov_len = 40960;

  int result = readv(fd, vec, 2);
  if(result == -1)
  {
    return -1;
  }
  else if(result <= writeable) 
  {
    buffer->writePos += result;
  }
  else 
  {
    buffer->writePos = buffer->capacity;
    bufferAppendData(buffer, temp, result - writeable);
  }

  free(temp);

  return result;
}
