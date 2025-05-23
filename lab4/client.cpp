#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int sockMain, serv_socket;
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
    int sectosleep = atoi(argv[2]);
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
        std::cout << "I send the: " << sectosleep << " after this seconds " << sectosleep <<'\n';
        if (send(sockMain, &sectosleep, 4, 0) < 0)
        {
            std::cerr << "Error on sending\n";
            close(sockMain);
            exit(1);
        }
        sleep(sectosleep);
    }
    close(sockMain);
    return 0;
}