class Buffer 
{
public:
  Buffer(int size);
  ~Buffer();
  
// 后面剩余的字节数
  int writeableSize();
// 未读的字节数
  int readableSize();

  void extendRoom(int size);

  int appendData(const char* data, int size);
  int appendString(const char* data);
  int socketRead(int fd);

private:
  char* _data;
  int _capacity;
  int _readPos;
  int _writePos;
};
