#include "tcpserver.hpp"

tcpserver::tcpserver(uint16_t port)
    : _port(port)
    , _listensock(-1)
{}

tcpserver::~tcpserver()
{
    if (_listensock >= 0)
    {
        close(_listensock);
        _listensock = -1;
    }
}

bool tcpserver::createSocket()
{
    _listensock = socket(AF_INET, SOCK_STREAM, 0);
    if (_listensock < 0)
    {
        perror("socket failed");
        return false;
    }

    return true;
}

bool tcpserver::setReuseAddr()
{
    int opt = 1;
    if (setsockopt(_listensock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        return false;
    }

    return true;
}

bool tcpserver::bindSocket()
{
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(_port);

    if (bind(_listensock, (struct sockaddr*)&local, sizeof(local)) < 0)
    {
        perror("bind failed");
        return false;
    }

    return true;
}

bool tcpserver::listenSocket(int backlog)
{
    if (listen(_listensock, backlog) < 0)
    {
        perror("listen failed");
        return false;
    }

    return true;
}

bool tcpserver::recvLine(int sockfd, string* out)
{
    out->clear();
    char ch = 0;

    while (true)
    {
        ssize_t n = recv(sockfd, &ch, 1, 0);
        if (n > 0)
        {
            if (ch == '\n')
            {
                return true;
            }
            if (ch != '\r')
            {
                out->push_back(ch);
            }
        }
        else if (n == 0)
        {
            return false;
        }
        else
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("recv failed");
            return false;
        }
    }
}

bool tcpserver::sendAll(int sockfd, const string& data)
{
    size_t total = 0;

    while (total < data.size())
    {
        ssize_t n = send(sockfd, data.c_str() + total, data.size() - total, 0);
        if (n > 0)
        {
            total += n;
        }
        else if (n < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            perror("send failed");
            return false;
        }
    }

    return true;
}

void tcpserver::service(int sockfd, const struct sockaddr_in& peer)
{
    char ipstr[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &peer.sin_addr, ipstr, sizeof(ipstr));
    uint16_t peer_port = ntohs(peer.sin_port);

    cout << "new client: " << ipstr << ":" << peer_port << endl;

    while (true)
    {
        string request;
        if (!recvLine(sockfd, &request))
        {
            cout << "client quit: " << ipstr << ":" << peer_port << endl;
            break;
        }

        cout << "[" << ipstr << ":" << peer_port << "] " << request << endl;

        string response = "server echo: ";
        response += request;
        response += "\n";

        if (!sendAll(sockfd, response))
        {
            break;
        }
    }

    close(sockfd);
}

bool tcpserver::init()
{
    if (!createSocket())
    {
        return false;
    }

    if (!setReuseAddr())
    {
        return false;
    }

    if (!bindSocket())
    {
        return false;
    }

    if (!listenSocket())
    {
        return false;
    }

    return true;
}

void tcpserver::run()
{
    for (;;)
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        memset(&peer, 0, sizeof(peer));

        int sockfd = accept(_listensock, (struct sockaddr*)&peer, &len);
        if (sockfd < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("accept failed");
            continue;
        }

        service(sockfd, peer);
    }
}

void tcpserver::start()
{
    if (_listensock < 0)
    {
        cerr << "listen socket not ready, please init first" << endl;
        return;
    }

    run();
}
