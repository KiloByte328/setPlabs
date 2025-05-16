#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

typedef struct{ // структура чтобы отличать какое сообщение нам прийдёт/мы отошлём
    int type; // 0 - новый логин, 1 - сообщение, 2 - send/recive clients, 3 - conection with someone, 4 - mediafiles, 5 - история чата, ответ 0 - нету, 6 - end of session
    std::size_t size; // отсылать media в два этапа? первый - размер, второй - название+тип данных(строка) ну и третий сама media
} message;

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::wcerr << L"Usage of programm is: ./cli <port>\n";
        exit(1);
    }
    message main_message;
    int sockMain, serv_socket;
    std::vector<std::wstring> other_clients;
    struct sockaddr_in serv;
    bool auth = false;
    std::wstring choise, typed_text;
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
        return 4;
    }
    std::wcout << L"i got connected with " << ntohs(serv.sin_port) << L"\n";
    if (recv(serv_socket, &main_message, sizeof(message), 0) < 0)
    {
        //exit
    }
    if (recv (serv_socket, &other_clients, main_message.size, 0) < 0)
    {
        //exit
    }
    while(1)
    {
        if (other_clients.size() != 0) {
        }
        if (!auth) {
            std::wcout << L"Enter your login\n";
            std::wcin >> lgn;
            std::wcout << L"Enter your password\n";
            std::wcin >>psw;
            auth = true;
            auto ans = std::find(other_clients.begin(), other_clients.end(), lgn);
            if (ans == other_clients.end()) {
                main_message.type = 0;
                main_message.size = sizeof(lgn);
                send(sockMain, &main_message, sizeof(main_message), 0);
                send(sockMain, &lgn, sizeof(lgn), 0);
                main_message.size = sizeof(psw);
                send(sockMain, &main_message, sizeof(main_message), 0);
                send(sockMain, &psw, sizeof(psw), 0);
                // send to server
            }
        }
        if (auth) {
            std::wcout << L"You are connected as: " << lgn << L'\n';
            for (std::size_t i = 0; i < other_clients.size(); i++) {
                std::wcout << L"client "  << i << L" : " << other_clients[i] << L"\n";
            }
            std::wcout << L"Choose chat or type 0 - logout\n";
            std::wcin >> choise;
            if(choise.compare(L"0") == 0) {
                lgn.clear();
                psw.clear();
                lgn = L"\0";
                psw = L"\0";
                auth = false;
            }
            for (auto& i : other_clients)
                if (i.compare(choise) == 0)
                    break;
                else {
                    std::wcout << "No User with this login was found, try again\n";
                    std::wcin >> choise;
                    i = other_clients[0];
                }
            // вывод истории
            main_message.type = 5;
            send(sockMain, &main_message, sizeof(main_message), 0);
            if (recv(sockMain, &main_message, sizeof(main_message), 0) == 0) {

            }
            if (recv(sockMain, &history_of_chat, main_message.size, 0) == 0) {

            }
            for (auto& i : history_of_chat) {
                std::wcout << i << '\n';
            }
            main_message.type = 1;
            std::wcout << L"Enter your next message\n";
            std::wcin >> typed_text;
            main_message.size = sizeof(typed_text);
            send(sockMain, &main_message, sizeof(main_message), 0);
            send(sockMain, &typed_text, sizeof(typed_text), 0);
        }
    }
    close(sockMain);
    return 0;
}