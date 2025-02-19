#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    int length, sockMain, cnct, buf;
    struct sockaddr_in serv, client;
    sockMain = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open socket" << '\n';
        return 1;
    }
    buf = 0;
    socklen_t size = sizeof(struct sockaddr);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = 0;
    client.sin_addr.s_addr = AF_INET;
    client.sin_family =INADDR_ANY;
    client.sin_port = 0;
    cnct = bind(sockMain, (struct sockaddr *) &serv, sizeof(serv));
    if (cnct == -1)
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
        int tmp = recvfrom(cnct, &buf, 4, 0, (struct sockaddr *) &client, &size);
        std::cout << ntohs(serv.sin_port) << " " << buf << '\n';
    }
    return 0;
}