#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
    int length, sockMain, cnct, buf, chk, clientSock, childsihave;
    struct sockaddr_in serv, client;
    childsihave = 0;
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
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
    socklen_t c_size = sizeof(client);
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
        if ((chk = listen(sockMain, 20)) == -1)
        {
            std::cerr << "error on listening" << '\n';
            return 4;
        }
        childsihave++;
        pid_t im = fork();
        switch (im)
        {
            case -1:
                std::cerr << "cant fork" << '\n';
                wait(&childsihave);
                return -1;
                break;
            case 0:
                if ((chk = listen(sockMain, 20)) == -1)
                {
                    std::cerr << "error on listening" << '\n';
                    return 4;
                }
                if ((clientSock = accept(sockMain, (struct sockaddr *) &client, &c_size)) == -1)
                {
                    std::cerr << "error while accepting" << '\n';
                    return 5;
                }
                std::cout << "I, the : " << im << " got connected with: " << ntohs(client.sin_port) << '\n';
                break;
            default:
        }
    }
    wait(&childsihave);
    return 0;
}