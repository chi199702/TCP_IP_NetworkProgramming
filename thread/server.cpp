#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <fcntl.h>
#include <error.h>
#include <pthread.h>

using namespace std;

#define BUF_SIZE 1024

void error_handling(string& msg);

void* processThread(void*);

int main (int argc, char* argv[]) {
    string error_msg = "";
    if (argc != 3) {
        error_msg = "input params : <ip> <port>";
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
        error_handling(error_msg);
    }

    while(1) {
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_sz = sizeof(clnt_addr);
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
        if (clnt_sock == -1) {
            continue;
        }
        // 创建线程,把该 sockt 丢给该线程处理
        pthread_t thread;   // unsigned long
        pthread_create(&thread, NULL, processThread, &clnt_sock);
        pthread_detach(thread);
        cout << "connected client : " << clnt_sock << endl;
    }
    return 0;
}

void* processThread(void* sock) {
    int* temp = (int*)sock;
    int clnt_sock = *temp;
    char msg[BUF_SIZE];
    while(1) {
        int msg_sz = read(clnt_sock, msg, BUF_SIZE);
        if (msg_sz <= 0) {
            break;
        }
        write(clnt_sock, msg, msg_sz);
    }
    close(clnt_sock);
    return NULL;
}

void error_handling(string& msg) {
    cout << msg << endl;
    exit(1);
}