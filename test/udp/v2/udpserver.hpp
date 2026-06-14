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

class udpserver
{
public:
    udpserver(uint16_t port);
    ~udpserver();

    bool init();
    void start();

private:
    bool createSocket();
    bool bindSocket();
    void run();

private:
    uint16_t _port;
    int _sock;
};
