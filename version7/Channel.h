#pragma once 
#include<functional>

// 回调函数的类型
using handleFunc = std::function<int(void*)>;
// 读写事件
enum FDEvent 
{
  ReadEvent = 0x02,
  WriteEvent = 0x04
};

class Channel 
{
public:
  Channel(int fd, int evets, handleFunc readCallback, handleFunc writeCallback, void* arg) ;
  void writeEventEnable(bool flag);
  bool isWriteEnanle();

private:
  int _fd;
  int  _evets;
  handleFunc _readCallback; 
  handleFunc _writeCallback;
  void* _arg;
};
