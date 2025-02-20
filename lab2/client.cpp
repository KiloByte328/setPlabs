#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int length, sockMain, serv_socket, buf;
    struct sockaddr_in serv, client;
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open socket" << '\n';
        close(serv_socket);
        close(sockMain);
        return 1;
    }
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
        close(serv_socket);
        close(sockMain);
        return 2;
    }
    if (getsockname(sockMain, (struct sockaddr *) &client, &size))
    {
        std::cerr << "cant get sock name" << '\n';
        close(serv_socket);
        close(sockMain);
        return 3;
    }
    std::cout << "my port is: " << ntohs(client.sin_port) << '\n';
    if ((serv_socket = connect(sockMain, (struct sockaddr*) &serv, s_size)) == -1)
    {
        std::cerr << "cant connect" << '\n';
        close(serv_socket);
        close(sockMain);
        return 4;
    }
    std::cout << "i got connected with " << ntohs(serv.sin_port) << '\n';
    while (1)
    {
        sleep(10);
        close(serv_socket);
        close(sockMain);
        exit(0);
    }
    return 0;
}