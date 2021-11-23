#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handle(char* error_msg);

/**
 * @brief 简单的回声客户端
 */
int main (int argc, char* argv[]) {
    // 判断运行时参数的个数
    if (argc != 3) {
        error_handle("input running params error");
    }

    // 1、创建“门卫” socket
    int serv_sock;
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        error_handle("create socket failure");
    }

    // 2、为 socket 绑定地址
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handle("socket bind error");
    }
    
    // 3、开启 socket 监听,最多接收 5 个连接请求
    if (listen(serv_sock, 5) == -1) {
        error_handle("socket listen error");
    }

    // 4、开始处理客户端
    int clnt_sock;                                      // 与客户端对接的 sokcet
    struct sockaddr_in clnt_addr;                       // 与服务器端连接的客户端地址
    socklen_t clnt_addr_size = sizeof(clnt_addr);       // 客户端地址结构的长度
    char msg[BUF_SIZE];
    for (int i = 0; i < 5; ++i) {
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) {
            error_handle("accept client error");
        }else {
            printf("Connected client %d \n", clnt_sock);
        }
        // 开始接收客户端的消息
        int recvMsgLength;
        while((recvMsgLength = read(clnt_sock, msg, BUF_SIZE - 1)) != 0) {
            write(clnt_sock, msg, recvMsgLength);
        }
        // 停止服务当前客户端
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}

void error_handle(char* error_msg) {
    fputs(error_msg, stderr);
    fputc('\n', stderr);
    exit(1);
}