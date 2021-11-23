#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

#define BUF_SIZE 1024

// 错误处理函数
void error_handling(string& msg);

void sig_handling(int);

/**
 * @brief 多进程服务器端
 * 进程和客户端是一对一的关系,利用信号处理防止僵尸进程
 * @return int 
 */
int main (int argc, char* argv[]) {
    string error_msg = "";
    if (argc != 3) {
        error_msg = "the number of command line params is error!";
        error_handling(error_msg);
    }

    // 创建 socket
    int serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        error_msg = "socket() error";
        error_handling(error_msg);
    }

    // 绑定地址
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    int bind_res = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bind_res == -1) {
        error_msg = "bind() error";
        error_handling(error_msg);
    }

    // 开启监听
    int listen_res = listen(serv_sock, 5);
    if (listen_res == -1) {
        error_msg = "listen() error";
    }

    // 注册信号——接收子进程
    struct sigaction act;
    act.sa_handler = sig_handling;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    // 开始从连接池中取出连接
    while(1) {
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_sz = sizeof(clnt_addr);

        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
        if (clnt_sock == -1) {
            continue;
        }
        cout << "accept client：" << clnt_sock << endl;

        int pid = fork();
        if (pid == 0) {
            // 子进程不需要门卫
            close(serv_sock);
            char recv_msg[BUF_SIZE];
            while (1) {
                int msg_sz = read(clnt_sock, recv_msg, BUF_SIZE);
                if (msg_sz == 0) {
                    cout << "clnt_sock close: " << clnt_sock << endl;
                    close(clnt_sock);
                    return 0;
                }
                write(clnt_sock, recv_msg, msg_sz);
            }
        }else {
            // 让子进程完全负责其拥有的 socket
            close(clnt_sock);
        }
    }
    close(serv_sock);
    return 0;
}

void error_handling(string& msg) {
    cout << msg << endl;
    exit(1);
}

// 信号处理函数——用户接收子进程
void sig_handling(int sig) {
    int statloc;
    pid_t pid = waitpid(-1, &statloc, WNOHANG);
    cout << "sub_pid close: " << pid << endl;
}