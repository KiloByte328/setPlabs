#include <iostream>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>

int main (int argc, char **argv)
{
    fd_set fd_in, fd_s;
    int sockMain, cnct, chk, clientSock, msocks, buf;
    struct sockaddr_in serv;
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open socket" << '\n';
        return 1;
    }
    socklen_t size = sizeof(struct sockaddr);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = 0;
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
    FD_ZERO(&fd_in);
    FD_ZERO(&fd_s);
    FD_SET(sockMain, &fd_s);
    FD_SET(sockMain, &fd_in);
    int allsocks = getdtablesize();
    msocks = 2;
    std::cout << "server port is: " << ntohs(serv.sin_port) << '\n';
    std::cout << "msocks is: " << msocks << '\n';
    while (1)
    {
        std::memcpy(&fd_in, &fd_s, sizeof(fd_in));
        if ((chk = listen(sockMain, 20)) == -1)
        {
            std::cerr << "error on listening" << '\n';
            return 4;
        }
        if (select(allsocks, &fd_in, NULL, NULL, NULL) < 0)
        {
            std::cerr << "cant select\n";
            return 6;
        }
        if (FD_ISSET(sockMain, &fd_in)) {
            if ((clientSock = accept(sockMain, (struct sockaddr *) 0, 0)) == -1)
            {
                std::cerr << "error while accepting" << '\n';
                return 5;
            }
            std::cout << "I was connected\n";
            FD_SET(clientSock, &fd_s);
            FD_SET(clientSock, &fd_in);
            msocks = fd_s.fds_bits[0];
        }
        for(int x = 0; x < msocks; x++)
        {
            if((x != sockMain) && (FD_ISSET(x, &fd_in)))
            {
                if (recv(x, &buf, 4, 0) == 0) {
                    std::cout << "So, my client close connection, so i will close connection too\n";
                    FD_CLR(x, &fd_s);
                    msocks = fd_s.fds_bits[0];
                    close(x);
                }
                std::cout << "i get: " << buf << "\n";
            }
        }
    }
    return 0;
}