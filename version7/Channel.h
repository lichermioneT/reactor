#pragma once 
#include<functional>

// 回调函数的类型
using handleFunc = std::function<int(void*)>;
// 读写事件
enum class FDEvent : unsigned char {ReadEvent = 0x01, WriteEvent = 0x2};

class Channel 
{
public:
  Channel(int fd, FDEvent evets, handleFunc readCallback, handleFunc writeCallback, void* arg) ;
  void writeEventEnable(bool flag);
  bool isWriteEnanle();

private:
  int _fd;
  FDEvent _evets;
  handleFunc _readCallback; 
  handleFunc _writeCallback;
  void* _arg;
};
