#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <fstream>
//#include <dpp/dpp.h>

/// @brief типы сообщений: клиентские: 
/// 0 - логин, 1 - сообщение, 2 - send/recive clients, 3 - conection as, 4 - connect with, 5 - история чата, ответ 0 - нету, 6 - end of session
/// типы сообщений: служебные:
/// 0 - новый логин(отправляет новый логин), 1 - удалить логин(отправляет удалённый логин), 2 - приватное сообщение для, 3 - залогинен как(отправляет логин)
typedef struct {
    int type;
    std::size_t size; // отсылать media в два этапа? первый - размер, второй - название+тип данных(строка) ну и третий сама media
} message;

//работает через внутренние сокеты и их пересвязку при создани нового дочернего процесса
int main()
{
    int sockMain = 0, clientSock, innersock = 0, insckin, cnct, chk, story_at = 0;
    fd_set fd_old, fd_upd;
    std::vector<int> f_recv, innersocks, clients, stry;
    std::vector<std::wstring> logins, passwords, chat_story;
    std::vector<sockaddr_in> clients_info;
    std::vector<pid_t> forks;
    std::size_t my_sock;
    std::wstring new_lgn, new_psw, msg, logined_as;
    struct sockaddr_in me, innernet;
    std::wfstream flstream;
    message main_message;
    if (socket(sockMain, SOCK_STREAM, 0) < 0)
    {
        std::wcerr << L"cant open socket\n";
        return 1;
    }
    if (socket(innersock, SOCK_STREAM, 0) < 0)
    {
        std::wcerr << L"cant open inner sock\n";
        return 32;
    }
    FD_ZERO(&fd_old);
    FD_ZERO(&fd_upd);
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
        std::wcerr << L"cant bind\n";
        return 2;
    }
    cnct = bind(innersock, (sockaddr *) &innernet, (socklen_t) sizeof(innernet));
    if (cnct == -1)
    {
        std::wcerr << L"cant bind inner socket\n";
        return 30;
    }
    FD_SET(sockMain, &fd_upd);
    FD_SET(innersock, &fd_upd);
    if (getsockname(sockMain, (sockaddr*) &me, (socklen_t*)sz))
    {
        std::wcerr << L"cant get sock name\n";
        return 3;
    }
    std::wcout << L"server port is: " << ntohs(me.sin_port) << L"\n";
    while(true)
    {
        std::memcpy(&fd_old, &fd_upd, sizeof(fd_upd));
        if ((chk = listen(sockMain, 100)) == -1)
        {
            std::wcerr << L"error on listening\n";
            for (int i = innersocks.size(); i <= 0; i--)
            {
                close(innersocks[i]);
                innersocks.pop_back();
            }
            close(clientSock);
            close(sockMain);
            return 4;
        }
        if (select(getdtablesize(), &fd_old, NULL, NULL, NULL) < 0) {
            std::wcerr << L"Can't do select\n";
            for(auto& i : innersocks) {
                close(i);
            }
            close(clientSock);
            close(sockMain);
            return 5;
        }
        if (FD_ISSET(sockMain, &fd_old)) {
            if ((clientSock = accept(sockMain, (struct sockaddr *) 0, 0)) == -1)
            {
                std::wcerr << L"error while accepting\n";
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
            main_message.type = 2;
            main_message.size = sizeof(clients);
            pid_t im = fork();
            switch(im)
            {
                case -1: 
                    std::wcerr << L"cant fork, end of work\n";
                    for (int i = innersocks.size(); i <= 0; i--)
                    {
                        close(innersocks[i]);
                        innersocks.pop_back();
                    }
                    close(clientSock);
                    close(sockMain);
                    return 33;
                // fork process lifetime
                case 0:
                    my_sock = clients.size();
                    // fd_set for_client;
                    // FD_ZERO(&for_client);
                    FD_ZERO(&fd_old);
                    //FD_ZERO(&fd_upd);
                    cnct = bind(innersock, (sockaddr *) &innernet, (socklen_t) sizeof(innernet));
                    if (cnct == -1)
                    {
                        std::wcerr << L"cant bind inner socket\n";
                        return 30;
                    }
                    if (connect(innersock, (sockaddr *) &me, sizeof(me)) < 0)
                    {
                        std::wcerr << L"cant connect to inner socket\n";
                        return 29;
                    }
                    FD_SET(innersock, &fd_old);
                    FD_SET(clients[my_sock], &fd_old);
                    if (select(getdtablesize(), &fd_old, NULL, NULL, NULL) < 0) {
                        close(clients[my_sock]);
                        close(innersock);
                        exit(1);
                        // exit
                    }
                    //std::memcpy(&fd_old, &fd_upd, sizeof(fd_upd));
                    while(true) {
                        if (FD_ISSET(innersock, &fd_old)) {
                            if (recv(innersock, &main_message ,sizeof(message), 0) == 0)
                            {
                                close(clients[my_sock]);
                                close(innersock);
                                exit(1);
                                // если родительский узел ничего не отправил, то остановить всю деятельность
                            }
                            switch (main_message.type) {
                                case 0:
                                    if (recv(innersock, &new_lgn, main_message.size, 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    logins.push_back(new_lgn);
                                    main_message.type = 2;
                                    main_message.size = sizeof(new_lgn);
                                    chat_story.push_back(new_lgn);
                                    story_at++;
                                    stry.push_back(1);
                                    // send(clients[my_sock], &main_message, sizeof(main_message), 0);
                                    // send(clients[my_sock], &new_lgn, sizeof(new_lgn), 0);
                                    break;
                                case 1:
                                    if (recv(innersock, &new_lgn, main_message.size, 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    chat_story.push_back(new_lgn);
                                    story_at++;
                                    stry.push_back(-1);
                                    // send(clientSock, &main_message,sizeof(main_message), 0);
                                    // send(clientSock, &new_lgn, sizeof(new_lgn), 0);
                                    for (std::size_t i = 0; i < logins.size(); i++) {
                                        if (logins[i].compare(new_lgn) == 0) {
                                            logins.erase(logins.begin() + i);
                                            break;
                                        }
                                    }
                                    break;
                                case 2:
                                    if (recv(innersock, &new_lgn ,main_message.size, 0) == 0)
                                    {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    if (recv(innersock, &main_message, sizeof(main_message), 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    if (recv(innersock, &msg, main_message.size, 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    if (logined_as.compare(new_lgn) == 0) {
                                        message to_client;
                                        to_client.type = 2;
                                        to_client.size = sizeof(msg) + sizeof(wchar_t) * msg.size();
                                        chat_story.push_back(msg);
                                        story_at++;
                                        stry.push_back(2);
                                        // send(clientSock, &to_client, sizeof(to_client), 0);
                                        // send(clientSock, &msg, sizeof(msg) + sizeof(wchar_t) * msg.size(), 0);
                                    }
                                    break;
                                case 3:
                                    if (recv(innersock, &new_lgn, main_message.size, 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    for (auto i = logins.begin(); i != logins.end(); i++) {
                                        if ((*i).compare(new_lgn) == 0) {
                                            logins.erase(i);
                                            break;
                                        }
                                    }
                                    //send(clients[my_sock], &main_message, sizeof(main_message), 0);
                                    //send(clients[my_sock], &new_lgn, sizeof(new_lgn), 0);
                                    break;
                            }
                        }
                        if (FD_ISSET(clients[my_sock], &fd_old)) {
                            message from_client;
                            if (recv(clients[my_sock], &from_client, sizeof(from_client), 0) == 0) {
                                std::wcerr << "Cant recv from client, shuting down";
                                from_client.type = 5;
                                send(innersock, &from_client, sizeof(from_client), 0);
                            }
                            switch (from_client.type) {
                                case -1:
                                    if (recv(clients[my_sock], &new_lgn, from_client.size, 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(2);
                                    }
                                    // if (recv(clients[my_sock], &main_message, sizeof(main_message), 0) == 0) {
                                    //     close(clients[my_sock]);
                                    //     close(innersock);
                                    //     exit(2);
                                    // }
                                    // if (recv(clients[my_sock], &new_psw, main_message.size, 0) == 0) {
                                    //     close(clients[my_sock]);
                                    //     close(innersock);
                                    //     exit(2);
                                    // }
                                    main_message.size = sizeof(new_lgn);
                                    send(innersock, &main_message, sizeof(main_message), 0);
                                    send(innersock, &new_lgn, sizeof(new_lgn), 0);
                                    // main_message.size = sizeof(new_psw);
                                    // send(innersock, &main_message, sizeof(main_message), 0);
                                    // send(innersock, &new_psw, sizeof(new_psw), 0);
                                    logins.push_back(new_lgn);
                                    //passwords.push_back(new_psw);
                                    break;
                                case 0:
                                    if (recv(clients[my_sock], &logined_as, from_client.size, 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    break;
                                case 1:
                                    if (recv(clients[my_sock], &msg, from_client.size, 0) == 0) {
                                        close(clients[my_sock]);
                                        close(innersock);
                                        exit(1);
                                    }
                                    break;
                                case 2:
                                    if (recv (clientSock, &msg, from_client.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    send(innersock, &main_message, sizeof(main_message), 0);
                                    send(innersock, &msg, main_message.size, 0);
                                    break;
                            }
                        }
                    }
                    break;
                default: {
                    if (listen(innersock, 100) < 0)
                    {
                        std::wcout << L"cant listen to inner sock\n";
                        for (int i = innersocks.size(); i <= 0; i--)
                        {
                            close(innersocks[i]);
                            innersocks.pop_back();
                        }
                        close(clientSock);
                        close(sockMain);
                        return 31;
                    }
                    if ((insckin = accept(innersock, (sockaddr *) &innernet, (socklen_t *) sizeof(innernet))) < 0)
                    {
                        std::wcerr << L"cant connect to inner socket\n";
                        close(clientSock);
                        close(sockMain);
                        return 30;
                    }
                    forks.push_back(im);
                    innersocks.push_back(insckin);
                    FD_SET(insckin, &fd_upd);
                    main_message.size = sizeof(clients_info);
                    for (std::size_t i = 0; i < innersocks.size(); i++)
                    {
                        send(innersocks[i], &main_message, sizeof(main_message), 0);
                        send(innersocks[i], &clients_info, sizeof(clients_info), 0);
                    }
                    break;
                }
            }
        }
        //if any of innersocks have some
        for (auto& in_socks : innersocks) {
            if (FD_ISSET(in_socks, &fd_old)) {
                if (recv(in_socks, &main_message, sizeof(main_message), 0) == 0) {
                    // exit
                }
                switch(main_message.type) {
                    case 0:
                        if (recv(in_socks, &new_lgn, main_message.size, 0) == 0) {
                            std::wcerr << "Can't recive from inner socket login info\n";
                            for (int i = innersocks.size(); i <= 0; i--)
                            {
                                close(innersocks[i]);
                                innersocks.pop_back();
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        // if (recv(in_socks, &main_message, sizeof(main_message), 0) == 0) {
                        //     std::wcerr << "Can't recive from inner socket password info\n";
                        //     for (int i = innersocks.size(); i <= 0; i--)
                        //     {
                        //         close(innersocks[i]);
                        //         innersocks.pop_back();
                        //     }
                        //     close(clientSock);
                        //     close(sockMain);
                        //     return 42;
                        // }
                        // if (recv(in_socks, &new_psw, main_message.size, 0) == 0) {
                        //     std::wcerr << "Can't recive from inner socket password info\n";
                        //     for (int i = innersocks.size(); i <= 0; i--)
                        //     {
                        //         close(innersocks[i]);
                        //         innersocks.pop_back();
                        //     }
                        //     close(clientSock);
                        //     close(sockMain);
                        //     return 42;
                        // }
                        logins.push_back(new_lgn);
                        // passwords.push_back(new_psw);
                        main_message.type = 0;
                        main_message.size = new_lgn.size();
                        for (auto& again : innersocks) {
                            if (again != in_socks) {
                                send(again, &main_message, sizeof(main_message), 0);
                                send(again, &new_lgn, sizeof(new_lgn), 0);
                            }
                        }
                        break;
                    case 1:
                        if (recv(in_socks, &new_lgn, main_message.size, 0) == 0) {
                            std::wcerr << "Can't recive from inner socket login info\n";
                            for (int i = innersocks.size(); i <= 0; i--)
                            {
                                close(innersocks[i]);
                                innersocks.pop_back();
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        for (std::size_t i = 0; i < logins.size(); i++) {
                            if (logins[i].compare(new_lgn) == 0) {
                                logins.erase(logins.begin() + i);
                            }
                        }
                        for (auto& i : innersocks) {
                            if (i != in_socks) {
                                send(i, &main_message, sizeof(main_message), 0);
                                send(i, &new_lgn, sizeof(new_lgn), 0);
                            }
                        }
                        break;
                    case 2:
                        if (recv(in_socks, &new_lgn, main_message.size, 0) == 0) {
                            for (int i = innersocks.size(); i <= 0; i--)
                            {
                                close(innersocks[i]);
                                innersocks.pop_back();
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        for (auto& i : clients) {
                            if (i != in_socks) {
                                send(i, &main_message, sizeof(main_message), 0);
                                send(i, &new_lgn, main_message.size, 0);
                            }
                        }
                        break;
                    // case 3:
                    //     if (recv(in_socks, &new_lgn, main_message.size, 0) == 0) {
                    //         for (int i = innersocks.size(); i <= 0; i--)
                    //         {
                    //             close(innersocks[i]);
                    //             innersocks.pop_back();
                    //         }
                    //         close(clientSock);
                    //         close(sockMain);
                    //         return 42;
                    //     }
                    //     for (auto& i : innersocks) {
                    //         if (i != in_socks) {
                    //             send(i, &main_message, sizeof(main_message), 0);
                    //             send(i, &new_lgn, main_message.size, 0);
                    //         }
                    //     }
                    //     for (auto i = logins.begin(); i != logins.end(); i++) {
                    //         if ((*i).compare(new_lgn) == 0) {
                    //             logins.erase(i);
                    //             break;
                    //         }
                    //     }
                    //     break;
                    default:
                        break;
                }
            }
        }
    }
}