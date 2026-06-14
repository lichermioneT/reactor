#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

class tcpserver
{
public:
    tcpserver(uint16_t port);
    ~tcpserver();

    bool init();
    void start();

private:
    bool createSocket();
    bool setReuseAddr();
    bool bindSocket();
    bool listenSocket(int backlog = 5);
    bool recvLine(int sockfd, string* out);
    bool sendAll(int sockfd, const string& data);
    void service(int sockfd, const struct sockaddr_in& peer);
    void run();

private:
    uint16_t _port;
    int _listensock;
};
