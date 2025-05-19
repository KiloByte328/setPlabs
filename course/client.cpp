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
#include <signal.h>
#include <termios.h>

termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); 
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int clear_screen ()
{
  std::cout << "\033[H\033[2J";
  return 0;
}

int get_screensize (int *cols, int *rows)
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
  std::string t;
  t.append("\033[");
  t.append(std::to_string(row));
  t.append(";");
  t.append(std::to_string(col));
  t.append("H");
  //printf ("\033[%d;%dH", row, col);
  std::cout << t << std::flush;
  return 0;
}

typedef struct {
    int type;
    std::size_t size;
} message;

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::wcerr << "Usage of programm is: ./cli <port>\n";
        exit(1);
    }
    int scrny, scrnx;
    get_screensize(&scrny, &scrnx);
    if (scrny < 3) {
        std::wcerr << "Cant open in this little window\n";
        exit(1);
    }
    std::ios::sync_with_stdio(false);
    fd_set ffd;
    message main_message;
    int sockMain, serv_socket;
    std::vector<std::string> other_clients;
    struct sockaddr_in serv;
    bool auth = false, newbi, private_message;
    std::string choise, typed_text, tmp_lgn, tmp_message, typed_message;
    std::vector<std::string> history_of_chat;
    std::string lgn = "\0", psw = "\0";
    char buf[100];
    sockMain = socket(AF_INET, SOCK_STREAM, 0);
    if (sockMain < 0)
    {
        std::cerr << "cant open socket" << '\n';
        close(sockMain);
        return 1;
    }
    FD_ZERO(&ffd);
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
    FD_SET(serv_socket, &ffd);
    std::cout << "i got connected with " << ntohs(serv.sin_port) << "\n";
    while(1)
    {
        if (!auth) {
            while (1) {
                std::cout << "If you want to exit chat write !0\nAre you a new User, if no y/n you will be a new User? y/n? \n";
                std::cin >> lgn;
                if (lgn.compare("!0") == 0) {
                    //
                    close(sockMain);
                    std::cout << "Goodbye!\n";
                    exit(0);
                }
                if (lgn.compare("y") == 0) {
                    newbi = true;
                    break;
                }
                if(lgn.compare("n")) {
                    break;
                }
            }
            std::cout << "Enter your login\n";
            std::cin >> lgn;
            auth = true;
            if (newbi) {
                main_message.type = 0;
                main_message.size = sizeof(char) * lgn.size();
                send(sockMain, &main_message, sizeof(main_message), 0);
                send(sockMain, lgn.c_str(), main_message.size, 0);
            }
            else {
                main_message.type = 3;
                main_message.size = sizeof(char) * lgn.size();
                send(sockMain, &main_message, sizeof(main_message), 0);
                send(sockMain, lgn.c_str(), main_message.size, 0);
            }
        }
        else {
            clear_screen();
            std::cout << "You are connected as: " << lgn << '\n';
            for (std::size_t i = 0; i < other_clients.size(); i++) {
                std::cout << "User "  << i << " : " << other_clients[i] << "\n";
            }
            std::cout << "Type !common to common chat, Username to private chat or type !0 to logout.\n";
            std::cin >> choise;
            if(choise.compare("!0") == 0) {
                main_message.type = -1;
                main_message.size = sizeof(char) * lgn.size() + 1;
                send(sockMain, &main_message, sizeof(main_message), 0);
                send(sockMain, lgn.c_str(), sizeof(char) * lgn.size(), 0);
                lgn.clear();
                psw.clear();
                lgn = "\0";
                psw = "\0";
                auth = false;
            }
            if (auth) {
                sleep(1);
                enable_raw_mode();
                if (choise.compare("!common") == 0) 
                    private_message = false;
                else {
                    for (auto& i : other_clients) {
                        if (i.compare(choise) == 0) {
                            break;
                        }
                        else if (i.compare(*other_clients.end()) == 0){
                            std::cout << "Can't found user " << choise << " Enter User login or enter !common\n";
                            std::cin >> choise;
                            if (choise.compare("!common") == 0) {
                                private_message = false;
                                break;
                            }
                            i = other_clients[0];
                        }
                    }
                    private_message = true;
                }
                while (1) {
                    typed_message.clear();
                    typed_text.clear();
                    cursorgoto(1, scrny - 1);
                    std::cout << "Enter your next message or type !0 to exit from chat\n" << std::flush;
                    disable_raw_mode();
                    std::cin >> typed_text;
                    enable_raw_mode();
                    if (typed_text.compare("!0") == 0) {
                        disable_raw_mode();
                        break;
                    }
                    if (private_message)
                        typed_message.append("Private message from ");
                    typed_message.append(lgn);
                    typed_message.append(" : ");
                    typed_message.append(typed_text);
                    typed_message.append("\n\0");
                    if (private_message) {
                        main_message.type = 2;
                        main_message.size = sizeof(char) * choise.size() + 1;
                        send(sockMain, &main_message, sizeof(main_message), 0);
                        send(sockMain, choise.c_str(), main_message.size, 0);
                        main_message.size = sizeof(char) * typed_message.size() + 1;
                        send(sockMain, &main_message, sizeof(main_message), 0);
                        send(sockMain, typed_message.c_str(), main_message.size, 0);
                    }   
                    else {
                        main_message.type = 1;
                        main_message.size = sizeof(char) * typed_message.size() + 1;
                        send(sockMain, &main_message, sizeof(main_message), 0);
                        send(sockMain, typed_message.c_str(), main_message.size, 0);
                    }
                    main_message.type = 4;
                    send(sockMain, &main_message, sizeof(main_message), 0);
                    for (std::size_t i = 0; main_message.size != i && main_message.type == 4; i++) {
                        if (recv(sockMain, &main_message, sizeof(main_message), 0) == 0) {
                            perror("Error on getting chat history");
                            close(sockMain);
                            exit(1);
                        }
                        if (main_message.type == 4) {
                            if (recv(sockMain, &buf, main_message.size, 0) == 0) {
                                perror("Error on getting chat history");
                                close(sockMain);
                                exit(1);
                            }
                            history_of_chat.push_back(buf);
                        }
                    }
                    clear_screen();
                    enable_raw_mode();
                    cursorgoto(1, 2);
                    disable_raw_mode();
                    std::cout << "You are loginnerd as: " << lgn << '\n';
                    int tmp = 0;
                    for (auto i = history_of_chat.begin(); tmp + 2 != scrny && i != history_of_chat.end(); i++, tmp++) {
                        //cursorgoto(1, 3 + tmp);
                        std::cout << *i;
                    }
                    std::cout << typed_message;
                    history_of_chat.push_back(typed_message);
                }
                disable_raw_mode();
            }
        }
    }
    close(sockMain);
    return 0;
}