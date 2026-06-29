#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "Buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

Buffer::Buffer(int size):_capacity(size > 0 ? size : 0)
{
    if (_capacity > 0)
    {
        _data = (char*)malloc(_capacity);
        if (_data != nullptr)
        {
            bzero(_data, _capacity);
        }
        else
        {
            _capacity = 0;
        }
    }
}

Buffer::~Buffer()
{
    if (_data != nullptr)
    {
        free(_data);
    }
}

bool Buffer::extendRoom(int size)
{
    if (size <= 0)
    {
        return true;
    }
    if (writeableSize() >= size)
    {
        return true;
    }
    if (_readPos + writeableSize() >= size)
    {
        int readable = readableSize();
        memmove(_data, _data + _readPos, readable);
        _readPos = 0;
        _writePos = readable;
        return true;
    }

    void* temp = realloc(_data, _capacity + size);
    if (temp == NULL)
    {
        return false;
    }
    memset((char*)temp + _capacity, 0, size);
    _data = static_cast<char*>(temp);
    _capacity += size;
    return true;
}

int Buffer::appendString(const char* data, int size)
{
    if (data == nullptr || size <= 0)
    {
        return -1;
    }
    // 扩容
    if (!extendRoom(size) || _data == nullptr)
    {
        return -1;
    }
    // 数据拷贝
    memcpy(_data + _writePos, data, size);
    _writePos += size;
    return 0;
}

int Buffer::appendString(const char* data)
{
    if (data == nullptr)
    {
        return -1;
    }
    int size = strlen(data);
    int ret = appendString(data, size);
    return ret;
}

int Buffer::appendString(const string& data)
{
    if (data.empty())
    {
        return 0;
    }
    int ret = appendString(data.data(), static_cast<int>(data.size()));
    return ret;
}

int Buffer::socketRead(int fd)
{
    struct iovec vec[2];
    char tmpbuf[40960];
    int writeable = writeableSize();
    vec[0].iov_base = writeable > 0 ? _data + _writePos : tmpbuf;
    vec[0].iov_len = writeable;
    vec[1].iov_base = tmpbuf;
    vec[1].iov_len = sizeof(tmpbuf);

    int result = readv(fd, vec, 2);
    if (result == -1)
    {
        return -1;
    }
    if (result <= writeable)
    {
        _writePos += result;
        return result;
    }

    _writePos = _capacity;
    if (appendString(tmpbuf, result - writeable) == -1)
    {
        return -1;
    }
    return result;
}

char* Buffer::findCRLF()
{
    if (readableSize() <= 0 || _data == nullptr)
    {
        return nullptr;
    }
    char* ptr = (char*)memmem(_data + _readPos, readableSize(), "\r\n", 2);
    return ptr;
}

int Buffer::sendData(int socket)
{
    // 判断有无数据
    int readable = readableSize();
    if (readable > 0)
    {
        int count = send(socket, _data + _readPos, readable, MSG_NOSIGNAL);
        if (count > 0)
        {
            _readPos += count;
            usleep(1);
        }
        return count;
    }
    return 0;
}

