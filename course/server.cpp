#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
//#include <dpp/dpp.h>

struct message{ // структура чтобы отличать какое сообщение нам прийдёт/мы отошлём
    int type; // 0 - message, 1 - send/recive clients, 2 - conection with someone 3 - ping, 4 - pong, 5 - media???
    std::size_t size; // отсылать media в два этапа? первый - размер, второй - название+тип данных(строка) ну и третий сама media
};

//работает через внутренние сокеты и их пересвязку при создани нового дочернего процесса
int main()
{
    int sockMain, clientSock, innersock, insckin, cnct, chk;
    std::vector<int> f_read, clients, innersocks;
    std::vector<sockaddr_in> clients_info;
    std::vector<pid_t> forks;
    struct sockaddr_in me, innernet;
    message main_message;
    if (socket(sockMain, SOCK_STREAM, 0) < 0)
    {
        std::cerr << "cant open socket\n";
        return 1;
    }
    if (socket(innersock, SOCK_STREAM, 0) < 0)
    {
        std::cerr << "cant open inner sock\n";
        return 32;
    }
    innernet.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    innernet.sin_family = AF_INET;
    innernet.sin_port = 0;
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = INADDR_ANY;
    me.sin_port = 0;
    auto sz = sizeof(me);
    cnct = bind(sockMain, (sockaddr*) &me, sz);
    if (cnct == -1)
    {
        std::cerr << "cant bind\n";
        return 2;
    }
    cnct = bind(innersock, (sockaddr *) &innernet, (socklen_t) sizeof(innernet));
    if (cnct == -1)
    {
        std::cerr << "cant bind inner socket\n";
        return 30;
    }
    if (getsockname(sockMain, (sockaddr*) &me, (socklen_t*)sz))
    {
        std::cerr << "cant get sock name" << '\n';
        return 3;
    }
    std::cout << "server port is: " << ntohs(me.sin_port) << '\n';
    while(true)
    {
        if ((chk = listen(sockMain, 20)) == -1)
        {
            std::cerr << "error on listening" << '\n';
            while(!forks.empty())
            {
                if (waitpid(*forks.end(), NULL, WNOHANG) != 0)
                {
                    forks.pop_back();
                }
            }
            for (int i = innersocks.size(); i <= 0; i--)
            {
                close(innersocks[i]);
                innersocks.pop_back();
            }
            close(clientSock);
            close(sockMain);
            return 4;
        }
        if ((clientSock = accept(sockMain, (struct sockaddr *) 0, 0)) == -1)
        {
            std::cerr << "error while accepting" << '\n';
            while(!forks.empty())
            {
                if (waitpid(*forks.end(), NULL, WNOHANG) != 0)
                {
                    forks.pop_back();
                }
            }
            for (int i = innersocks.size(); i <= 0; i--)
            {
                close(innersocks[i]);
                innersocks.pop_back();
            }
            close(clientSock);
            close(sockMain);
            return 5;
        }
        clients.push_back(clientSock);
        sockaddr_in tmp;
        getsockname(clientSock, (sockaddr*) &tmp, (socklen_t *) sizeof(sockaddr_in));
        clients_info.push_back(tmp);
        main_message.type = 0;
        main_message.size = sizeof(clients);
        pid_t im = fork();
        switch(im)
        {
            case -1: 
                std::cerr << "cant fork, end of work\n";
                while(!forks.empty())
                {
                    if (waitpid(*forks.end(), NULL, WNOHANG) != 0)
                    {
                        forks.pop_back();
                    }
                }
                for (int i = innersocks.size(); i <= 0; i--)
                {
                    close(innersocks[i]);
                    innersocks.pop_back();
                }
                close(clientSock);
                close(sockMain);
                return 33;
            case 0:
                message get_next_message;
                fd_set f_refresh;
                cnct = bind(innersock, (sockaddr *) &innernet, (socklen_t) sizeof(innernet));
                if (cnct == -1)
                {
                    std::cerr << "cant bind inner socket\n";
                    return 30;
                }
                if (connect(innersock, (sockaddr *) &me, sizeof(me)) < 0)
                {
                    std::cerr << "cant connect to inner socket\n";
                    return 29;
                }
                while(true) {
                    if (recv(innersock, &get_next_message ,sizeof(message), 0) == 0)
                    {
                        // если родительский узел ничего не отправил, то остановить всю деятельность
                    }
                    if (recv(innersock, &clients_info ,get_next_message.size, 0) == 0)
                    {
                        // если родительский узел ничего не отправил, то остановить всю деятельность
                    }
                    message to_client;
                    to_client.type = 1;
                    to_client.size = sizeof(clients_info);
                    send(clientSock, &to_client, sizeof(to_client), 0);
                    send(clientSock, &clients_info, sizeof(clients_info), 0);
                }
                break;
            default:
                if (listen(innersock, 20) < 0)
                {
                    std::cout << "cant listen to inner sock\n";
                    while(!forks.empty())
                    {
                        if (waitpid(*forks.end(), NULL, WNOHANG) != 0)
                        {
                            forks.pop_back();
                        }
                    }
                    for (int i = innersocks.size(); i <= 0; i--)
                    {
                        close(innersocks[i]);
                        innersocks.pop_back();
                    }
                    close(clientSock);
                    close(sockMain);
                    return 31;
                }
                if (insckin = accept(innersock, (sockaddr *) &innernet, (socklen_t *) sizeof(innernet)) < 0)
                {
                    std::cerr << "cant connect to inner socket\n";
                    while(!forks.empty())
                    {
                        if (waitpid(*forks.end(), NULL, WNOHANG) != 0)
                        {
                            forks.pop_back();
                        }
                    }
                    for (int i = innersocks.size(); i <= 0; i--)
                    {
                        close(innersocks[i]);
                        innersocks.pop_back();
                    }
                    close(clientSock);
                    close(sockMain);
                    return 30;
                }
                forks.push_back(im);
                innersocks.push_back(insckin);
                main_message.size = sizeof(clients_info);
                for (int i = 0; i < innersocks.size(); i++)
                {
                    send(innersocks[i], &main_message, sizeof(main_message), 0);
                    send(innersocks[i], &clients_info, sizeof(clients_info), 0);
                }
/*                 main_message.type = 3;
                for (auto i = clients.begin(); i != clients.end(); i++)
                {
                    message check;
                    send(*i, &main_message, sizeof(main_message), 0);
                    if (recv(*i, &check, sizeof(message), 0) == 0 && check.type != 4)
                    {
                        close(*i);
                        clients.erase(i);
                    }
                } */
        }
    }
}