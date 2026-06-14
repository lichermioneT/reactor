# 编译器
CXX = g++
# 编译选项：生成调试信息，开启警告
CXXFLAGS = -g -Wall

# 目标文件
TARGETS = client server

# 默认构建所有目标
all: $(TARGETS)

# 链接 client
client: client.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# 链接 server
server: server.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# 清理生成的文件
clean:
	rm -f $(TARGETS)

# 伪目标
.PHONY: all clean
