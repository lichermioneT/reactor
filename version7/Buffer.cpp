#include "Buffer.h"
#include <cstring>
#include <sys/uio.h>

Buffer::Buffer(int size)
  :_data(new char[size])
  ,_capacity(size)
  ,_readPos(0)
  ,_writePos(0)
{}

Buffer::~Buffer()
{
  if(_data)
  {
    delete[] _data;
  }
}

// 后面还剩的字节数
int Buffer::writeableSize()
{
  return _capacity - _writePos;
}

// 未读的字节数
int Buffer::readableSize()
{
  return _writePos - _readPos;
}

void Buffer::extendRoom(int size)
{
// 1.后面的空间足够
    if(writeableSize() >= size)
    {
        return;
    }

    int readable = readableSize();

// 2.前面的废弃空间 + 后面的空间够用
    if(writeableSize() + _readPos >= size)
    {
        memmove(_data, _data + _readPos, readable);

        _readPos = 0;
        _writePos = readable;
    }
    else
    {
// 3.真正扩容
        int newCapacity = _capacity + size;

        char* temp = new char[newCapacity];

        memcpy(temp, _data + _readPos, readable);

        delete[] _data;
        _data = temp;

        _readPos = 0;
        _writePos = readable;
        _capacity = newCapacity;
    }
}

int Buffer::appendData(const char* data, int size)
{
  if(data == nullptr ||  size <= 0)
  {
    return -1;
  }

  extendRoom(size);
  memcpy(_data+_writePos, data, size);
  _writePos += size;

  return  size;
}

int Buffer::appendString(const char* data)
{
  int size = strlen(data);
  return  appendData(data, size);
}

int Buffer::socketRead(int fd)
{
    int writable = writeableSize();

    char temp[40960];

    struct iovec vec[2];

    vec[0].iov_base = _data + _writePos;
    vec[0].iov_len = writable;

    vec[1].iov_base = temp;
    vec[1].iov_len = sizeof(temp);

    ssize_t ret = readv(fd, vec, 2);

    if(ret < 0)
    {
        return -1;
    }

    if(ret <= writable)
    {
        _writePos += ret;
    }
    else
    {
        _writePos = _capacity;

        appendData(temp, ret - writable);
    }

    return ret;
}
