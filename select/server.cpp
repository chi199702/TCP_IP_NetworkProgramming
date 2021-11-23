#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/select.h>
#include <sys/time.h>

using namespace std;

#define BUF_SIZE 1024

void error_handling(string& msg);

int main (int argc, char* argv[]) {
    string error_msg = "";
    if (argc != 3) {
        error_msg = "input params should be : <ip> <port>";
        error_handling(error_msg);
    }

    // 创建 socket
    int serv_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_socket == -1) {
        error_msg = "socket() error";
        error_handling(error_msg);
    }

    // 绑定地址
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    int bind_res = bind(serv_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // 开启监听
    int listen_res = listen(serv_socket, 5);
    if (listen_res == -1) {
        error_msg = "listen() error";
        error_handling(error_msg);
    }

    // 设置好存放客户端 socket 的 fd_set 变量
    fd_set read_fdset;
    FD_ZERO(&read_fdset);
    FD_SET(serv_socket, &read_fdset);     // 监听门卫 socket
    int maxfd = serv_socket + 1;
    
    // 对于每个 socket 都可以使用的信息可以放在循环体外
    char msg[BUF_SIZE];
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_sz = sizeof(clnt_addr);
    int clnt_sock;

    // 开始调用 select
    while (1) {
        fd_set temp = read_fdset;         // 备份原变量!
        int fd_num = select(maxfd, &temp, NULL, NULL, NULL);
        if (fd_num == -1) {
            error_msg = "select() error";
            error_handling(error_msg);
        }else{  // 由于没有设置超时，所以不会返回 0
            // 查看 [0, maxfd) 范围内的 socket 是否有变化
            for (int fd = 0; fd < maxfd; ++fd) {
                if (FD_ISSET(fd, &temp)) {
                    if(fd == serv_socket) {     // 门卫 socket
                        clnt_sock = accept(fd, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
                        FD_SET(clnt_sock, &read_fdset);
                        maxfd = clnt_sock + 1;
                    }else {
                        int msg_sz = read(fd, msg, BUF_SIZE);
                        if (msg_sz <= 0) {  // 客户端 socket 已经关闭,服务器端对应的 socket 应该关闭
                            close(fd);
                            FD_CLR(fd, &read_fdset);
                            continue;
                        }
                        msg[msg_sz] = 0;
                        write(fd, msg, msg_sz);
                    }
                }
            }
        }
    }
    close(serv_socket);
}

void error_handling(string& msg) {
    cout << msg;
    exit(1);
}