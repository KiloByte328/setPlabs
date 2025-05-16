#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <sys/ioctl.h>

int clear_screen ()
{
  printf ("\033[H\033[2J");
  return 0;
}

int get_screensize (int *rows, int *cols)
{
  struct winsize ws;
  if (ioctl (1, TIOCGWINSZ, &ws))
    return -1;
  *rows = ws.ws_row;
  *cols = ws.ws_col;
  return 0;
};

int cursorgoto (int col, int row)
{
  if ((col == 0) || (row == 0))
    return -1;
  int X, Y;
  if (get_screensize (&Y, &X) == 1)
    return -1;
  if ((Y < row) || (X < col))
    return -1;
  printf ("\033[%d;%dH", row, col);
  return 0;
}

typedef struct { // структура чтобы отличать какое сообщение нам прийдёт/мы отошлём
    int type; // -1 - end of session, 0 - новый логин, 1 - залогинен как существующий, 2 - private message
    std::size_t size; // отсылать media в два этапа? первый - размер, второй - название+тип данных(строка) ну и третий сама media
} message;

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::wcerr << L"Usage of programm is: ./cli <port>\n";
        exit(1);
    }
    int scrny, scrnx;
    get_screensize(&scrny, &scrnx);
    if (scrny < 2) {
        std::wcerr << L"Cant open in this little window\n";
        exit(1);
    }
    fd_set ffd;
    message main_message;
    int sockMain, serv_socket;
    std::vector<std::wstring> other_clients;
    struct sockaddr_in serv;
    bool auth = false;
    bool newbi;
    std::wstring choise, typed_text, tmp_lgn, tmp_message, typed_message;
    std::vector<std::wstring> history_of_chat;
    std::wstring lgn = L"\0", psw = L"\0";
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
    if (sockMain < 0)
    {
        std::wcerr << "cant open socket" << '\n';
        close(sockMain);
        return 1;
    }
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(atoi(argv[1]));
    socklen_t s_size = sizeof(serv);
    if ((serv_socket = connect(sockMain, (struct sockaddr*) &serv, s_size)) == -1)
    {
        std::wcerr << "cant connect" << '\n';
        close(sockMain);
        close(serv_socket);
        return 4;
    }
    std::wcout << L"i got connected with " << ntohs(serv.sin_port) << L"\n";
    if (recv(serv_socket, &main_message, sizeof(main_message), 0) < 0)
    {
        close(serv_socket);
        close(sockMain);
        exit(1);
        //exit
    }
    if (recv (serv_socket, &other_clients, main_message.size, 0) < 0)
    {
        close(serv_socket);
        close(sockMain);
        exit(1);
        //exit
    }
    if (select(getdtablesize(), &ffd, NULL, NULL, NULL) < 0) {
        //exit
        close(serv_socket);
        close(sockMain);
        exit(1);
    }
    while(1)
    {
        if (!auth) {
            while (1) {
                std::wcout << "Are you a new User? y/n?\n";
                std::wcin >> lgn;
                if (lgn.compare(L"y") || lgn.compare(L"n")) {
                    newbi = lgn.compare(L"y") == 0 ? true : false;
                }
            }
            std::wcout << L"Enter your login\n";
            std::wcin >> lgn;
            //std::wcout << L"Enter your password\n";
            //std::wcin >>psw;
            auth = true;
            if (newbi) {
                main_message.type = -1;
                main_message.size = sizeof(lgn) + sizeof(wchar_t) * lgn.size();
                send(serv_socket, &main_message, sizeof(main_message), 0);
                send(serv_socket, &lgn, sizeof(lgn) + sizeof(wchar_t) * lgn.size(), 0);
                // main_message.size = sizeof(psw);
                // send(serv_socket, &main_message, sizeof(main_message), 0);
                // send(serv_socket, &psw, sizeof(psw), 0);
                // send to server
            }
            else {
                // password checking need to be here
                // if ()
                main_message.type = 0;
                main_message.size = sizeof(lgn) + sizeof(wchar_t) * lgn.size();
                send(serv_socket, &main_message, sizeof(main_message), 0);
                send(serv_socket, &lgn, sizeof(lgn) + sizeof(wchar_t) * lgn.size(), 0);
            }
        }
        else {
            clear_screen();
            std::wcout << L"You are connected as: " << lgn << L'\n';
            for (std::size_t i = 0; i < other_clients.size(); i++) {
                std::wcout << L"User "  << i << L" : " << other_clients[i] << L"\n";
            }
            std::wcout << L"Choose chat with user to private chat or type !common to common chat or type !0 to logout\n";
            std::wcin >> choise;
            if(choise.compare(L"!0") == 0) {
                main_message.type = -1;
                main_message.size = sizeof(lgn) + sizeof(wchar_t) * lgn.size();
                send(serv_socket, &main_message, sizeof(main_message), 0);
                send(serv_socket, &lgn, sizeof(lgn) + sizeof(wchar_t) * lgn.size(), 0);
                lgn.clear();
                psw.clear();
                lgn = L"\0";
                psw = L"\0";
                auth = false;
                break;
            }
            if (auth) {
                if (choise.compare(L"!common") == 0) {
                    while (1) {
                        // вывод истории
                        // main_message.type = 5;
                        // send(sockMain, &main_message, sizeof(main_message), 0);
                        // if (recv(sockMain, &main_message, sizeof(main_message), 0) == 0) {
                        // }
                        // if (recv(sockMain, &history_of_chat, main_message.size, 0) == 0) {
                        // }
                        // for (auto& i : history_of_chat) {
                        //     std::wcout << i << '\n';
                        // }
                        if (FD_ISSET(serv_socket, &ffd)) {
                            if (recv(serv_socket, &main_message, sizeof(main_message), 0) == 0) {
                                close(sockMain);
                                close(serv_socket);
                                exit(1);
                                //exit
                            }
                            switch(main_message.type) {
                                case -1:
                                    if (recv(serv_socket, &tmp_lgn, main_message.size, 0) == 0) {
                                        close(sockMain);
                                        close(serv_socket);
                                        exit(1);
                                        //exit
                                    }
                                    for(std::size_t i = 0; i < other_clients.size(); i++) {
                                        if (other_clients[i].compare(tmp_lgn) == 0) {
                                            other_clients.erase(other_clients.begin() + i);
                                            break;
                                        }
                                    }
                                    std::wcout << L"User " << tmp_lgn << L" exit chat\n";
                                    //someone exit
                                    break;
                                case 0:
                                    if (recv(serv_socket, &tmp_lgn, main_message.size, 0) == 0) {
                                        close(sockMain);
                                        close(serv_socket);
                                        exit(1);
                                        //exit
                                    }
                                    other_clients.push_back(tmp_lgn);
                                    std::wcout << L"User " << tmp_lgn << L" has joined chat\n";
                                    //someone new
                                    break;
                                case 1:
                                    // сервер считает на сколько мы отстали
                                    while (main_message.type != 3) {
                                        if (recv(serv_socket, &tmp_message, main_message.size, 0) == 0) {
                                            close(sockMain);
                                            close(serv_socket);
                                            exit(1);
                                            //exit
                                        }
                                        history_of_chat.push_back(tmp_message);
                                        if (recv(serv_socket, &main_message, sizeof(main_message), 0) == 0) {
                                            close(sockMain);
                                            close(serv_socket);
                                            exit(1);
                                            //exit
                                        }
                                    }
                                    for (auto& i : history_of_chat) {
                                        std::wcout << i; 
                                    }
                                    //new messages
                                    break;
                            }
                        }
                        main_message.type = 1;
                        std::wcout << L"Enter your next message or type !0 to exit from chat\n";
                        std::wcin >> typed_text;
                        if (typed_text.compare(L"!0") == 0) {
                            break;
                        }
                        typed_message.append(lgn);
                        typed_message.append(L" : ");
                        typed_message.append(typed_text);
                        typed_message.append(L"\n");
                        main_message.type = 1;
                        main_message.size = sizeof(typed_message) + sizeof(wchar_t) * typed_message.size();
                        send(serv_socket, &main_message, sizeof(main_message), 0);
                        send(serv_socket, &typed_message, sizeof(typed_message), 0);
                    }
                }
                else {
                    for (auto& i : other_clients) {
                        if (i.compare(choise) == 0) {
                            break;
                        }
                        else if (i.compare(*other_clients.end()) == 0){
                            std::wcout << L"Can't found user " << choise << L" Enter User login\n";
                            std::wcin >> choise;
                            i = other_clients[0];
                        }
                    }
                    while (1) {
                        if (FD_ISSET(serv_socket, &ffd)) {
                            if (recv(serv_socket, &main_message, sizeof(main_message), 0) == 0) {
                                close(sockMain);
                                close(serv_socket);
                                exit(1);
                                //exit
                            }
                            switch(main_message.type) {
                                case -1:
                                    if (recv(serv_socket, &tmp_lgn, main_message.size, 0) == 0) {
                                        close(sockMain);
                                        close(serv_socket);
                                        exit(1);
                                        //exit
                                    }
                                    for(std::size_t i = 0; i < other_clients.size(); i++) {
                                        if (other_clients[i].compare(tmp_lgn) == 0) {
                                            other_clients.erase(other_clients.begin() + i);
                                            break;
                                        }
                                    }
                                    std::wcout << L"User " << tmp_lgn << L" exit chat\n";
                                    //someone exit
                                    break;
                                case 0:
                                    if (recv(serv_socket, &tmp_lgn, main_message.size, 0) == 0) {
                                        close(sockMain);
                                        close(serv_socket);
                                        exit(1);
                                        //exit
                                    }
                                    other_clients.push_back(tmp_lgn);
                                    std::wcout << L"User " << tmp_lgn << L" has joined chat\n";
                                    //someone new
                                    break;
                                case 1:
                                    // сервер считает на сколько мы отстали
                                    while (main_message.type == 1) {
                                        if (recv(serv_socket, &tmp_message, main_message.size, 0) == 0) {
                                            close(sockMain);
                                            close(serv_socket);
                                            exit(1);
                                            //exit
                                        }
                                        history_of_chat.push_back(tmp_message);
                                        if (recv(serv_socket, &main_message, sizeof(main_message), 0) == 0) {
                                            close(sockMain);
                                            close(serv_socket);
                                            exit(1);
                                            //exit
                                        }
                                    }
                                    for (auto& i : history_of_chat) {
                                        std::wcout << i; 
                                    }
                                    //new messages
                                    break;
                            }
                        }
                        main_message.type = 1;
                        std::wcout << L"Enter your next message or type !0 to exit from chat\n";
                        std::wcin >> typed_text;
                        if (typed_text.compare(L"!0") == 0) {
                            break;
                        }
                        typed_message.append(lgn);
                        typed_message.append(L" : ");
                        typed_message.append(typed_text);
                        typed_message.append(L"\n");
                        main_message.type = 2;
                        main_message.size = sizeof(typed_message) + sizeof(wchar_t) * typed_message.size();
                        send(serv_socket, &main_message, sizeof(main_message), 0);
                        send(serv_socket, &typed_message, sizeof(typed_message), 0);
                    }
                    // todo maybe not do as ip, do as server passing only to someone, checking at server
                }
            }
        }
    }
    close(sockMain);
    return 0;
}