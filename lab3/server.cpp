#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <pthread.h>


pthread_mutex_t g_mut;
pthread_barrier_t bar;

int s_func(int sock)
{
    //pthread_mutexattr_t m_atrbs;
    pthread_mutex_init(&g_mut, NULL);
/*     std::fstream output("output.txt", std::ios::app); */
    int buf;
    while (1) 
    {
    int tmp = recv(sock, &buf, 4, 0);
    std::cout << "I have recived: " << tmp << " bytes\n";
    if (tmp == 0)
    {
        std::cout << "Now this thread will be free\n";
        pthread_mutex_unlock(&g_mut);
        pthread_exit(0);
    }
    std::cout << "I recive: " << buf << " Now i gonna put it in file!\n";
    pthread_mutex_lock(&g_mut);
/*     output << buf << '\n'; */
    FILE* fp;
    if (fp = fopen("output.txt", "a"))
    {
        std::cout << fp << '\n';
    }
    fprintf(fp, "%d\n", buf);
    fclose(fp);
    pthread_mutex_unlock(&g_mut);
    sleep(buf);
    std::cout << "now i put it in file and sand it back: " << buf << '\n';
    send(sock, &buf, 4, 0);
    }
}

int main (int argc, char **argv)
{
    int sockMain, cnct, chk, clientSock, chlds;
    pthread_t threads[20];
    pthread_attr_t ti;
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
    pthread_attr_init(&ti);
    pthread_attr_setdetachstate(&ti, /* PTHREAD_CREATE_JOINABLE */ PTHREAD_CREATE_DETACHED);
    pthread_barrier_init(&bar, NULL, 6);
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
        pthread_create(&threads[chlds], &ti, (void*(*)(void*)) s_func, (void *)clientSock);
        chlds++;
        std::cout << "Now i have: " << chlds << " this many childs\n";
        if(chlds >= 5)
        {
            pthread_barrier_wait(&bar);
/*             for(int x = 0; x < chlds; x++)
            {
                std::cout << "now im waiting: " << x << '\n';
                pthread_join(threads[x], NULL);
            } */
            std::cout << "Change da world, i accompish the mission! Goodbye\n";
            pthread_attr_destroy(&ti);
            pthread_barrier_destroy(&bar);
            return 0;
        }
    }
    pthread_attr_destroy(&ti);
    return 0;
}