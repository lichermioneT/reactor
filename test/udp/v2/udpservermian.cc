#include "udpserver.hpp"
#include <cstdlib>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " port" << endl;
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));

    udpserver svr(port);
    if (!svr.init())
    {
        return 2;
    }

    svr.start();
    return 0;
}
