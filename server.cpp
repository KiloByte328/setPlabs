#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    int length, sockMain;
    struct sockaddr_in serv, client;
    sockMain = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open" << '\n';
        return 1;
    }
    socklen_t size = sizeof(struct sockaddr);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = 0;
    if (bind(sockMain, (struct sockaddr *) &serv, sizeof(serv)))
    {
        std::cerr << "cant bind" << '\n';
        return 2;
    }
    if (getsockname(sockMain, (struct sockaddr *) &serv, &size))
    {
        std::cerr << "cant get sock name" << '\n';
        return 3;
    }
    std::cout << "server port is: " << ntohs(serv.sin_port) << '\n';
    while (1)
    {
        //recivefrom();
    }
    return 0;
}