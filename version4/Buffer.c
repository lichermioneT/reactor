#define _GNU_SOURCE

#include "Buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

struct Buffer* bufferInit(int size)
{
  if(size <= 0)
  {
    return NULL;
  }

  struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
  if(buffer == NULL)
  {
    perror("malloc");
    return NULL;
  }

  buffer->data = (char*)malloc((size_t)size);
  if(buffer->data == NULL)
  {
    perror("malloc");
    free(buffer);
    return NULL;
  }

  buffer->capacity = size;
  buffer->readPos = 0;
  buffer->writePos = 0;
  memset(buffer->data, 0, (size_t)size);

  return buffer;
}

void bufferDestory(struct Buffer* buffer)
{
  if(buffer != NULL)
  {
    free(buffer->data);
    free(buffer);
  }
}

// MODIFIED: restored the lost if/else logic and return failure to callers.
int bufferExtendrRoom(struct Buffer* buffer, int size)
{
  if(buffer == NULL || size <= 0)
  {
    return -1;
  }

  if(bufferWriateableSize(buffer) >= size)
  {
    return 0;
  }

  if(buffer->readPos + bufferWriateableSize(buffer) >= size)
  {
    int readable = bufferReadableSize(buffer);
    memmove(buffer->data, buffer->data + buffer->readPos, (size_t)readable);
    buffer->readPos = 0;
    buffer->writePos = readable;
    return 0;
  }

  int newCapacity = buffer->capacity + size;
  char* temp = (char*)realloc(buffer->data, (size_t)newCapacity);
  if(temp == NULL)
  {
    perror("realloc");
    return -1;
  }

  memset(temp + buffer->capacity, 0, (size_t)(newCapacity - buffer->capacity));
  buffer->data = temp;
  buffer->capacity = newCapacity;
  return 0;
}

int bufferWriateableSize(struct Buffer* buffer)
{
  if(buffer == NULL)
  {
    return 0;
  }

  return buffer->capacity - buffer->writePos;
}

int bufferReadableSize(struct Buffer* buffer)
{
  if(buffer == NULL)
  {
    return 0;
  }

  return buffer->writePos - buffer->readPos;
}

int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
  if(buffer == NULL || data == NULL || size <= 0)
  {
    return -1;
  }

  if(bufferExtendrRoom(buffer, size) != 0)
  {
    return -1;
  }

  memcpy(buffer->data + buffer->writePos, data, (size_t)size);
  buffer->writePos += size;

  return 0;
}

int bufferAppendString(struct Buffer* buffer, const char* data)
{
  if(data == NULL)
  {
    return -1;
  }

  return bufferAppendData(buffer, data, (int)strlen(data));
}

int bufferSocketRead(struct Buffer* buffer, int fd)
{
  if(buffer == NULL || fd < 0)
  {
    return -1;
  }

  struct iovec vec[2];
  int writeable = bufferWriateableSize(buffer);

  vec[0].iov_base = buffer->data + buffer->writePos;
  vec[0].iov_len = (size_t)writeable;

  char* tmpbuf = (char*)malloc(40960);
  if(tmpbuf == NULL)
  {
    perror("malloc");
    return -1;
  }

  vec[1].iov_base = tmpbuf;
  vec[1].iov_len = 40960;

  int result = (int)readv(fd, vec, 2);
  if(result == -1)
  {
    free(tmpbuf);
    return -1;
  }

  if(result <= writeable)
  {
    buffer->writePos += result;
  }
  else
  {
    buffer->writePos = buffer->capacity;
    bufferAppendData(buffer, tmpbuf, result - writeable);
  }

  free(tmpbuf);
  return result;
}

char* bufferFindCRLF(struct Buffer* buffer)
{
  if(buffer == NULL)
  {
    return NULL;
  }

  return (char*)memmem(buffer->data + buffer->readPos,
                      (size_t)bufferReadableSize(buffer), "\r\n", 2);
}

int bufferSendData(struct Buffer* buffer, int socket)
{
  int readable = bufferReadableSize(buffer);
  if(readable <= 0)
  {
    if(buffer != NULL)
    {
      buffer->readPos = 0;
      buffer->writePos = 0;
    }
    return 0;
  }

  int count = (int)send(socket, buffer->data + buffer->readPos, (size_t)readable, 0);
  if(count > 0)
  {
    buffer->readPos += count;
    if(buffer->readPos == buffer->writePos)
    {
      buffer->readPos = 0;
      buffer->writePos = 0;
    }
    usleep(1);
  }

  return count;
}
