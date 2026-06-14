#include "udpserver.hpp"

udpserver::udpserver(uint16_t port)
    : _port(port)
    , _sock(-1)
{}

udpserver::~udpserver()
{
    if (_sock >= 0)
    {
        close(_sock);
        _sock = -1;
    }
}

bool udpserver::createSocket()
{
    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sock < 0)
    {
        perror("socket failed");
        return false;
    }

    return true;
}

bool udpserver::bindSocket()
{
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));

    local.sin_family = AF_INET;
    local.sin_port = htons(_port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(_sock, (struct sockaddr*)&local, sizeof(local));
    if (ret < 0)
    {
        perror("bind failed");
        return false;
    }

    return true;
}

bool udpserver::init()
{
    if (!createSocket())
    {
        return false;
    }

    if (!bindSocket())
    {
        return false;
    }

    return true;
}

void udpserver::run()
{
    char buffer[1024];

    for (;;)
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        memset(&peer, 0, sizeof(peer));
        memset(buffer, 0, sizeof(buffer));

        ssize_t n = recvfrom(_sock,
                             buffer,
                             sizeof(buffer) - 1,
                             0,
                             (struct sockaddr*)&peer,
                             &len);
        if (n < 0)
        {
            perror("recvfrom failed");
            continue;
        }

        buffer[n] = '\0';

        string client_ip = inet_ntoa(peer.sin_addr);
        uint16_t client_port = ntohs(peer.sin_port);

        cout << "[" << client_ip << ":" << client_port << "] "
             << buffer << endl;

        string message = "server get message: ";
        message += buffer;

        ssize_t s = sendto(_sock,
                           message.c_str(),
                           message.size(),
                           0,
                           (struct sockaddr*)&peer,
                           len);
        if (s < 0)
        {
            perror("sendto failed");
            continue;
        }
    }
}

void udpserver::start()
{
    if (_sock < 0)
    {
        cerr << "socket not ready, please init first" << endl;
        return;
    }

    run();
}
