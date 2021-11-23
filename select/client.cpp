#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

using namespace std;

#define BUF_SIZE 1024

void error_handling(string& msg);

int main (int argc, char* argv[]) {
    string error_msg = "";
    if (argc != 3) {
        error_msg = "command line param : <ip> <port>";
        error_handling(error_msg);
    }

    // 创建 socket
    int clnt_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (clnt_socket == -1) {
        error_msg = "socket() error";
        error_handling(error_msg);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    int con_res = connect(clnt_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (con_res == -1) {
        error_msg = "connect() error";
        error_handling(error_msg);
    }

    char msg[BUF_SIZE];
    while (1) {
        cout << "input message(q/Q to quit) : ";
        fgets(msg, BUF_SIZE - 1, stdin);    // fgets 会在末尾补 0
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            break;
        }
        write(clnt_socket, msg, BUF_SIZE);
        int msg_sz = read(clnt_socket, msg, BUF_SIZE);
        if (msg_sz <= 0) {
            cout << "disconnect from server" << endl;
            break;
        }
        msg[msg_sz] = 0;
        fputs(msg, stdout);
    }
    close(clnt_socket);
}

void error_handling(string& msg) {
    cout << msg << endl;;
    exit(1);
}