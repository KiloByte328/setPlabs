#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

void rpr(int sign)
{
    int stat;
    while(wait3(&stat, WNOHANG, (struct rusage *)0) >= 0);
}

int main (int argc, char **argv)
{
    int sockMain, cnct, chk, clientSock, stat, chlds;
    struct sockaddr_in serv;
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open socket" << '\n';
        return 1;
    }
    chlds = 0;
    socklen_t size = sizeof(struct sockaddr);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = 0;
    signal(SIGCHLD, rpr);
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
        if ((chk = listen(sockMain, 5)) == -1)
        {
            std::cerr << "error on listening" << '\n';
            return 4;
        }
        if ((clientSock = accept(sockMain, (struct sockaddr *) 0, 0)) == -1)
        {
            std::cerr << "error while accepting" << '\n';
            return 5;
        }
        std::cout << "I was connected!" << '\n';
        chlds++;
        pid_t im = fork();
        switch (im)
        {
            case -1:
                std::cerr << "cant fork" << '\n';
                wait(&stat);
                return -1;
                break;
            case 0:
                while (1) {
                    int buf = 0;
                    ssize_t tmp = 0;
                    tmp = recv(clientSock, &buf, 4, 0);
                    std::cout << "i recive: " << buf << '\n';
                    if (tmp == 0)
                    {
                        close(clientSock);
                        exit(0);
                    }
                    std::cout << "I got the: " << buf << " and i send it back! " << "\n";
                    send(clientSock, &buf, 4, 0);
                }
                break;
            default:
            if(chlds == 5)
            {
                wait(&stat);
                close(sockMain);
                close(clientSock);
                exit(0);
            }
            break;
        }
    }
    return 0;
}