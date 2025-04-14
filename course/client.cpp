#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>

struct message{ // структура чтобы отличать какое сообщение нам прийдёт/мы отошлём
    int type; // 0 - message, 1 - send/recive clients, 2 - conection with someone 3 - ping, 4 - pong, 5 - media???
    std::size_t size; // отсылать media в два этапа? первый - размер, второй - название+тип данных(строка) ну и третий сама media
};

int main(int argc, char **argv)
{
    message first;
    int sockMain, serv_socket;
    std::vector<sockaddr_in> other_clients;
    struct sockaddr_in serv;
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open socket" << '\n';
        close(sockMain);
        return 1;
    }
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(atoi(argv[1]));
    socklen_t s_size = sizeof(serv);
    if ((serv_socket = connect(sockMain, (struct sockaddr*) &serv, s_size)) == -1)
    {
        std::cerr << "cant connect" << '\n';
        close(sockMain);
        return 4;
    }
    std::cout << "i got connected with " << ntohs(serv.sin_port) << '\n';
    while(1)
    {
        if (recv(serv_socket, &first, sizeof(message), 0) < 0)
        {
            //exit
        }
        if (recv (serv_socket, &other_clients, first.size, 0) < 0)
        {
            //exit
        }

    }
    close(sockMain);
    return 0;
}