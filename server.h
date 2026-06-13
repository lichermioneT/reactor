#pragma once
#include <stdint.h>

// 初始化监听的套接字
int initListenFd(uint16_t port);
// 启动epoll
int epollrun(int lfd);
// 和客户端建立链接
int acceptClient(int epfd, int lfd);
