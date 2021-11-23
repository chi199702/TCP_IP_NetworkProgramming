#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define BUF_SIZE 1024

void error_handling(string& msg);

int main (int argc, char* argv[]) {
    string error_msg = "";
    // 检查命令行参数
    if (argc != 3) {
        error_msg = "the number of command line params error";
        error_handling(error_msg);
    }

    // 创建客户端 socket
    int clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (clnt_sock == -1) {
        error_msg = "socket() error";
        error_handling(error_msg);
    }
    
    // 指定服务器端地址并连接
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    int con_res = connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (con_res == -1) {
        error_msg = "connect() error";
        error_handling(error_msg);
    }

    // 发接消息
    while (1) {
        char msg[BUF_SIZE];
        fgets(msg, BUF_SIZE - 1, stdin);
        if (!strcmp(msg, "Q\n") || !strcmp(msg, "q\n")) {
            break;
        }
        int write_sz = write(clnt_sock, msg, BUF_SIZE);

        int read_sz = read(clnt_sock, msg, BUF_SIZE);
        if (read_sz == 0) {
            error_msg = "read() error";
            error_handling(error_msg);
        }
        msg[read_sz] = 0;
        cout << msg;
    }
    close(clnt_sock);
    return 0;
}

void error_handling(string& msg) {
    cout << msg << endl;
}