#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    int length, sockMain, numb, buf;
    struct sockaddr_in serv, client;
    sockMain = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open socket" << '\n';
        return 1;
    }
    numb = atoi(argv[2]);
    socklen_t size = sizeof(struct sockaddr);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(atoi(argv[1]));
    socklen_t s_size = sizeof(serv);
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = 0;
    int cnct = bind(sockMain, (struct sockaddr *) &client, sizeof(client));
    if (cnct == -1)
    {
        std::cerr << "cant bind" << '\n';
        return 2;
    }
    if (getsockname(sockMain, (struct sockaddr *) &client, &size))
    {
        std::cerr << "cant get sock name" << '\n';
        return 3;
    }
    std::cout << "my port is: " << ntohs(client.sin_port) << '\n';
    while (1)
    {
        int tmp = sendto(sockMain, &numb, 4, 0, (struct sockaddr*) &serv, s_size);
        std::cout << "first" << ntohs(client.sin_port) << " " << ntohs(serv.sin_port) << " " << numb << '\n';
        tmp = recvfrom(sockMain, &buf, 4, 0, (struct sockaddr*) &serv, &s_size);
        std::cout << "second " << ntohs(client.sin_port) << " " << ntohs(serv.sin_port) << " " << buf << '\n';
    }
    return 0;
}