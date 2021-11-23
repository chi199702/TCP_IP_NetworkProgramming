#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1025

void error_handle(char *error_msg) {
    fputs(error_msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

/**
 * @brief 连接简单的回声服务器端
 */
int main (int argc, char* argv[]) {
    // 判断运行时参数个数
    if (argc != 3) {
        error_handle("input running params error");
    }

    // 1、创建客户端 socket
    int clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (clnt_sock == -1) {
        error_handle("create client socket occur error");
    }

    // 2、连接服务器端
    struct sockaddr_in serv_addr;                       // 服务器端地址
    memset((void*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    int con_res = connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (con_res == -1) {
        error_handle("connect server error");
    }

    // 3、发送数据
    char msg[BUF_SIZE];
    while(1) {
        fputs("input message(Q to quit): ", stdout);
        fgets(msg, BUF_SIZE - 1, stdin);                // 从控制台接收数据
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            break;
        }

        ssize_t data_sz = write(clnt_sock, msg, strlen(msg));
        
        int recv_sz = 0;
        while (recv_sz < data_sz) {
            int one_recv = read(clnt_sock, &msg[recv_sz], BUF_SIZE - 1);
            if (one_recv == 0) {
                error_handle("read() error");
            }
            recv_sz += one_recv;
        }
        msg[recv_sz] = 0;
        fputs(msg, stdout);
    }
    close(clnt_sock);
    return 0;
}