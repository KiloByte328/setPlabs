#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <fstream>
//#include <dpp/dpp.h>

/// @brief типы сообщений: клиентские: 
/// 0 - новый логин(отправляет новый логин), 1 - сообщение для всех, 2 - приватное сообщение, 3 - залогинен как(отправляет логин)
typedef struct {
    int type;
    std::size_t size; // отсылать media в два этапа? первый - размер, второй - название+тип данных(строка) ну и третий сама media
} message;

//работает через внутренние сокеты и их пересвязку при создани нового дочернего процесса
int main()
{
    std::string path = "/tmp/msk";
    int sockMain = 0, clientSock, insckin, cnct, chk;
    fd_set fd_old, fd_upd;
    std::vector<int> f_recv, innersocks, clients, stry;
    std::vector<message> queue;
    std::vector<std::string> logins, passwords, msgs, tmp_lgns, history_of_chat;
    std::vector<sockaddr_in> clients_info;
    std::vector<pid_t> forks;
    std::string new_lgn, new_psw, msg, logined_as;
    struct sockaddr_in me, f_test, innernet;
    std::wfstream flstream;
    message main_message;
    char buf[100], duf[100];
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
    unlink(path.c_str());
    if (sockMain < 0)
    {
        std::cerr << "can't open socket\n";
        return 1;
    }
    FD_ZERO(&fd_old);
    FD_ZERO(&fd_upd);
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    me.sin_port = 0;
    auto sz = sizeof(me);
    cnct = bind(sockMain, (sockaddr*) &me, sz);
    if (cnct == -1)
    {
        std::cerr << "cant bind\n";
        return 2;
    }
    FD_SET(sockMain, &fd_upd);
    socklen_t sz2 = sizeof(f_test);
    if (getsockname(sockMain, (sockaddr*) &f_test, &sz2))
    {
        perror("getsockname");
        std::cerr << "can't get sock name\n";
        return 3;
    }
    std::wcout << "server port is: " << ntohs(f_test.sin_port) << "\n";
    while(true)
    {
        std::memcpy(&fd_old, &fd_upd, sizeof(fd_upd));
        if ((chk = listen(sockMain, 100)) == -1)
        {
            std::cerr << "error on listening\n";
            for (auto& i : innersocks)
            {
                close(innersocks[i]);
                innersocks.pop_back();
            }
            close(clientSock);
            close(sockMain);
            return 4;
        }
        if (select(4096, &fd_old, NULL, NULL, NULL) < 0) {
            std::cerr << "Can't do select\n";
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
                std::cerr << "error while accepting\n";
                for (auto& i : innersocks)
                {
                    close(innersocks[i]);
                    innersocks.pop_back();
                }
                close(clientSock);
                close(sockMain);
                return 5;
            }
            innernet.sin_family = AF_INET;
            innernet.sin_addr.s_addr = IN_LOOPBACKNET;
            innernet.sin_port = 0;
            int innersock = socket(AF_INET, SOCK_STREAM, 0);
            if (innersock < 0)
            {
                std::cerr << "can't open inner sock\n";
                return 32;
            }
            clients.push_back(clientSock);
            sockaddr_in tmp;
            getsockname(clientSock, (sockaddr*) &tmp, (socklen_t *) sizeof(sockaddr_in));
            clients_info.push_back(tmp);
            cnct = bind(innersock, (sockaddr*) &innernet, sizeof(sockaddr_in));
            if (cnct == -1) {
                perror("bind fai");
                close(sockMain);
                for (auto& i : innersocks) {
                    close(i);
                }
                exit(1);
            }
            getsockname(innersock, (sockaddr *) &tmp, &sz2);
            main_message.type = 2;
            main_message.size = sizeof(clients);
            pid_t im = fork();
            switch(im)
            {
                case -1: 
                    std::cerr << "cant fork, end of work\n";
                    for (auto& i : innersocks)
                    {
                        close(innersocks[i]);
                    }
                    close(clientSock);
                    close(sockMain);
                    return 33;
                // fork process lifetime
                case 0:
                    innersock = socket(AF_INET, SOCK_STREAM, 0);
                    if (innersock == -1) {
                        perror("fork socket fail ");
                        close(clientSock);
                        exit(1);
                    }
                    FD_ZERO(&fd_old);
                    innernet.sin_port = tmp.sin_port;
                    if (connect(innersock, (sockaddr *) &innernet, sizeof(innernet)) < 0)
                    {
                        perror("fork can't connet ");
                        close(clientSock);
                        close(innersock);
                        return 29;
                    }
                    FD_SET(innersock, &fd_upd);
                    FD_SET(clientSock, &fd_upd);
                    while(true) {
                        std::memcpy(&fd_old, &fd_upd, sizeof(fd_upd));
                        if (select(1024, &fd_old, NULL, NULL, NULL) < 0) {
                            close(clientSock);
                            close(innersock);
                            exit(1);
                        }
                        if (FD_ISSET(innersock, &fd_old)) {
                            //FD_CLR(innersock, &fd_old);
                            if (recv(innersock, &main_message ,sizeof(message), 0) == 0) {
                                close(clientSock);
                                close(innersock);
                                exit(1);
                            }
                            switch (main_message.type) {
                                case 0:
                                    if (recv(innersock, &buf, main_message.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    new_lgn.insert(0, buf, main_message.size);
                                    logins.push_back(new_lgn);
                                    main_message.type = 0;
                                    main_message.size = sizeof(char) * new_lgn.size();
                                    msg.clear();
                                    msg.insert(0, buf, main_message.size);
                                    msg.append(" joined chat\n");
                                    history_of_chat.push_back(msg);
                                    // send(clientSock, &main_message, sizeof(main_message), 0);
                                    // send(clientSock, new_lgn.c_str(), sizeof(char) * new_lgn.size(), 0);
                                break;
                                case 1:
                                    if (recv(innersock, &buf, main_message.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    msg.clear();
                                    msg.insert(0, buf, main_message.size);
                                    history_of_chat.push_back(msg);
                                    // send(clientSock, &main_message,sizeof(main_message), 0);
                                    // send(clientSock, msg.c_str(), main_message.size, 0);
                                break;
                                case 2:
                                    if (recv(innersock, &buf ,main_message.size, 0) == 0)
                                    {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    new_lgn.insert(0, buf, main_message.size);
                                    if (recv(innersock, &main_message, sizeof(main_message), 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    if (recv(innersock, &duf, main_message.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    msg.insert(0, duf, main_message.size);
                                    if (logined_as.compare(new_lgn) == 0) {
                                        message to_client;
                                        to_client.type = 2;
                                        to_client.size = sizeof(char) * msg.size();
                                        history_of_chat.push_back(buf);
                                        //send(clientSock, &to_client, sizeof(to_client), 0);
                                        //send(clientSock, msg.c_str(), sizeof(char) * msg.size(), 0);
                                    }
                                break;
                                case 3:
                                    if (recv(innersock, &buf, main_message.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    new_lgn.insert(0, buf, main_message.size);
                                    for (auto i = logins.begin(); i != logins.end(); i++) {
                                        if ((*i).compare(new_lgn) == 0) {
                                            logins.erase(i);
                                            break;
                                        }
                                    }
                                    main_message.type = -1;
                                    msg.clear();
                                    msg.insert(0, buf, main_message.size);
                                    msg.append(" leave the chat\n");
                                    //send(clientSock, &main_message, sizeof(main_message), 0);
                                    //send(clientSock, new_lgn.c_str(), main_message.size, 0);
                                break;
                            }
                        }
                        if (FD_ISSET(clientSock, &fd_old)) {
                            //FD_CLR(clientSock, &fd_old);
                            message from_client;
                            if (recv(clientSock, &from_client, sizeof(from_client), 0) == 0) {
                                std::cerr << "Cant recv from client, shuting down\n";
                                from_client.type = 5;
                                send(innersock, &from_client, sizeof(from_client), 0);
                                close(clientSock);
                                close(innersock);
                                exit(1);
                            }
                            switch (from_client.type) {
                                case 0:
                                    if (recv(clientSock, &buf, from_client.size, 0) == 0) {
            
                                        close(clientSock);
                                        close(innersock);
                                        exit(2);
                                    }
                                    new_lgn.insert(0, buf, from_client.size);
                                    send(innersock, &from_client, sizeof(from_client), 0);
                                    send(innersock, new_lgn.c_str(), from_client.size, 0);
                                    logins.push_back(new_lgn);
                                break;
                                case 1:
                                    if (recv(clientSock, &buf, from_client.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    msg.insert(0, buf, from_client.size);
                                    send(innersock, &from_client, sizeof(from_client), 0);
                                    send(innersock, msg.c_str(), from_client.size, 0);
                                    break;
                                case 2:
                                    if (recv (clientSock, &buf, from_client.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    new_lgn.insert(0, buf, main_message.size);
                                    if (recv(clientSock, &from_client, sizeof(from_client), 0) == 0) {

                                    }
                                    if (recv(clientSock, &duf, from_client.size, 0) == 0) {

                                    }
                                    msg.insert(0, duf, from_client.size);
                                    from_client.size = new_lgn.size();
                                    send(innersock, &from_client, sizeof(from_client), 0);
                                    send(innersock, new_lgn.c_str(), from_client.size, 0);
                                    from_client.size = msg.size();
                                    send(innersock, &from_client, sizeof(from_client), 0);
                                    send(innersock, msg.c_str(), from_client.size, 0);
                                    break;
                                case 3:
                                    if (recv(clientSock, &buf, from_client.size, 0) == 0) {
                                        close(clientSock);
                                        close(innersock);
                                        exit(1);
                                    }
                                    logined_as.insert(0, buf, from_client.size);
                                    break;
                                case 4:
                                    main_message.type = 4;
                                    main_message.size = history_of_chat.size();
                                    for (auto& i : history_of_chat) {
                                        main_message.size = i.size() * sizeof(char) + 1;
                                        send(clientSock, &main_message, sizeof(main_message), 0);
                                        send(clientSock, i.c_str(), main_message.size, 0);
                                    }
                                    from_client.type = 0;
                                    send(clientSock, &from_client, sizeof(from_client), 0);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    break;
                default: {
                    if (listen(innersock, 100) == -1)
                    {
                        std::wcout << "main cant listen to inner sock\n";
                        for (auto& i : innersocks)
                        {
                            close(innersocks[i]);
                        }
                        close(clientSock);
                        close(sockMain);
                        return 31;
                    }
                    if ((insckin = accept(innersock, (sockaddr *) &innernet, &sz2)) == -1)
                    {
                        std::cerr << "main cant connect to inner socket\n";
                        close(clientSock);
                        close(sockMain);
                        return 30;
                    }
                    forks.push_back(im);
                    innersocks.push_back(insckin);
                    FD_SET(insckin, &fd_upd);
                    main_message.size = sizeof(clients_info);
                    for (auto& i : innersocks)
                    {
                        if (i != insckin) {
                            send(i, &main_message, sizeof(main_message), 0);
                            send(i, &clients_info, sizeof(clients_info), 0);
                        }
                    }
                    break;
                }
            }
        }
        //if any of innersocks have some
        for (auto& in_socks : innersocks) {
            if (FD_ISSET(in_socks, &fd_old)) {
                // FD_CLR(in_socks, &fd_old);
                if (recv(in_socks, &main_message, sizeof(main_message), 0) == 0) {
                    std::cerr << "Can't recive information from inner socket\n";
                    for (auto& i : innersocks)
                    {
                        close(i);
                    }
                    close(clientSock);
                    close(sockMain);
                    exit(1);
                    // exit
                }
                //queue.push_back(main_message);
                switch(main_message.type) {
                    case 0:
                        if (recv(in_socks, &buf, main_message.size, 0) == 0) {
                            std::cerr << "Can't recive from inner socket login info\n";
                            for (auto& i : innersocks)
                            {
                                close(i);
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        new_lgn.insert(0, buf, main_message.size);
                        //tmp_lgns.push_back(buf);
                        for (auto& again : innersocks) {
                            if (again != in_socks && (send(again, nullptr, 1, MSG_PEEK) != 0)) {
                                send(again, &main_message, sizeof(main_message), 0);
                                send(again, new_lgn.c_str(), main_message.size, 0);
                            }
                        }
                        break;
                    case 1:
                        if (recv(in_socks, &buf, main_message.size, 0) == 0) {
                            std::cerr << "Can't recive message from inner\n";
                            for (auto& i : innersocks)
                            {
                                close(i);
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        msg.insert(0, buf, main_message.size);
                        //msgs.push_back(&buf);
                        for (auto& i : innersocks) {
                            if (i != in_socks && (send(i, nullptr, 1, MSG_PEEK) != 0)) {
                                send(i, &main_message, sizeof(main_message), 0);
                                send(i, buf, sizeof(new_lgn), 0);
                            }
                        }
                        break;
                    case 2:
                        if (recv(in_socks, &buf, main_message.size, 0) == 0) {
                            for (auto& i : innersocks)
                            {
                                close(i);
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        new_lgn.insert(0, buf, main_message.size);
                        //tmp_lgns.push_back(&buf);
                        if (recv(in_socks, &main_message, sizeof(main_message), 0) == 0) {
                            for (auto& i : innersocks)
                            {
                                close(i);
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        //queue.push_back(main_message);
                        if (recv(in_socks, &duf, main_message.size, 0) == 0) {
                            for (auto& i : innersocks)
                            {
                                close(i);
                            }
                            close(clientSock);
                            close(sockMain);
                            return 42;
                        }
                        msg.insert(0, duf, main_message.size);
                        //msgs.push_back(&buf);
                        for (auto& i : clients) {
                            if (i != in_socks && (send(i, nullptr, 1, MSG_PEEK) != 0)) {
                                main_message.size = new_lgn.size() * sizeof(char);
                                send(i, &main_message, sizeof(main_message), 0);
                                send(i, buf, main_message.size, 0);
                                main_message.size = msg.size() * sizeof(char);
                                send(i, &main_message, sizeof(main_message), 0);
                                send(i, duf, main_message.size, 0);
                            }
                        }
                        break;
                    case 5:
                        for(auto i = innersocks.begin(); i != innersocks.end(); i++) {
                            if (*i == in_socks) {
                                FD_CLR(*i, &fd_upd);
                                innersocks.erase(i);
                                break;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}