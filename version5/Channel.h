#pragma once

// 事件回调函数类型
using handleFunc = int(*)(void*);

// 文件描述符事件类型
enum class FDEvent
{
  ReadEvent = 0x02,
  WriteEvent = 0x04
};

// Channel:封装fd,关注事件，回调函数和回调函数参数
class Channel
{
private:
  int _fd;
  int _events;
  void* _arg;

public:
  handleFunc _readCallback;
  handleFunc _writeCallback;
  handleFunc _destroyCallback;

public:
  Channel(int fd, int events, void*arg, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryCallback);

  // 开启或者关闭写事件
  void writeEventEnable(bool flag);

  // 判断是否监听写事件
  bool isWriteEventEnable() const; 

public:
  inline int getSocket() const
  {
    return _fd;
  }

  inline int getEvent()const
  {
    return _events;
  }

  inline const void* getArg() const
  {
    return _arg;
  }
};
