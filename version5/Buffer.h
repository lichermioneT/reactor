#pragma once
#include <string>
using namespace std;

class Buffer
{
public:
    Buffer(int size);
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // 扩容
    bool extendRoom(int size);
    // 得到剩余的可写的内存容量
    inline int writeableSize()
    {
        return _capacity - _writePos;
    }
    // 得到剩余的可读的内存容量
    inline int readableSize()
    {
        return _writePos - _readPos;
    }
    // 写内存 1. 直接写 2. 接收套接字数据
    int appendString(const char* data, int size);
    int appendString(const char* data);
    int appendString(const string& data);
    int socketRead(int fd);
    // 根据\r\n取出一行, 找到其在数据块中的位置, 返回该位置
    char* findCRLF();
    // 发送数据
    int sendData(int socket);    // 指向内存的指针
    // 得到读数据的起始位置
    inline char* data()
    {
        return _data + _readPos;
    }
    inline int readPosIncrease(int count)
    {
        if (count <= 0)
        {
            return _readPos;
        }
        _readPos += count;
        if (_readPos > _writePos)
        {
            _readPos = _writePos;
        }
        return _readPos;
    }
private:
    char* _data = nullptr;
    int _capacity = 0;
    int _readPos = 0;
    int _writePos = 0;
};

