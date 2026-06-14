# 定义编译器和编译选项，方便以后统一修改
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# 默认目标，执行 make 时会同时编译服务端和客户端
.PHONY: all
all: udpserver udpclient

# 编译服务端 (注意：这里用的是你目前的拼写 udpservermian.cc)
udpserver: udpservermian.cc udpserver.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

# 编译客户端
udpclient: udpclientmain.cc udpclient.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

# 清理生成的可执行文件
.PHONY: clean
clean:
	rm -f udpserver udpclient
