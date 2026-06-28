#pragma once

struct Buffer
{
  char* data;
  int capacity;
  int readPos;
  int writePos;
};

// MODIFIED: restored buffer declarations that were previously swallowed by comments.
struct Buffer* bufferInit(int size);
void bufferDestory(struct Buffer* buffer);
int bufferExtendrRoom(struct Buffer* buffer, int size);
int bufferWriateableSize(struct Buffer* buffer);
int bufferReadableSize(struct Buffer* buffer);
int bufferAppendData(struct Buffer* buffer, const char* data, int size);
int bufferAppendString(struct Buffer* buffer, const char* data);
int bufferSocketRead(struct Buffer* buffer, int fd);
char* bufferFindCRLF(struct Buffer* buffer);
int bufferSendData(struct Buffer* buffer, int socket);
