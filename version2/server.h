#pragma once
#include <stdint.h>

// 初始化监听的套接字
int initListenFd(uint16_t port);
// 启动epoll
int epollrun(int lfd);
// 和客户端建立链接
// int acceptClient(int epfd, int lfd);
void* acceptClient(void* arg);
// 接收http请求
// int recvHttpRequest(int cfd, int epfd);
void* recvHttpRequest(void* arg);
// 请求行的解析
int parseRequestLine(const char* line, int cfd);
// 发送文件
int sendFile(const char* fileName, int cfd);
// 发送响应头(状态行 + 响应头)
int sendHeadMsg(int cfd, int status, const char* descr, const char* type, int length);
const char* getFileType(const char* name);
// 发送目录
int sendDir(const char* dirName, int cfd);

int hexToDec(char c);
void decodeMsg(char* to, char* from);
