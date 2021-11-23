#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <fcntl.h>
#include <error.h>

using namespace std;

#define EPOLL_SIZE 5
#define BUF_SIZE 4

void set_nonblock(int sockfd);
void error_handling(string& msg);

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

    // 开始 epoll 例程
    int epfd = epoll_create(10);

    // 添加门卫 socket 到 epoll 例程
    struct epoll_event event;
    event.events = EPOLLIN;     // serv_sock 不适用边缘触发方法
    event.data.fd = serv_sock;
    set_nonblock(serv_sock);
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    struct epoll_event* fds;
    fds = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    // 开始监听 epoll 例程
    while(1) {
        int fd_num = epoll_wait(epfd, fds, EPOLL_SIZE, -1);
        for (int i = 0; i < fd_num; ++i) {
            if (fds[i].data.fd == serv_sock) {      // 门卫 socket 收到通知
                struct sockaddr_in clnt_addr;
                socklen_t clnt_addr_sz = sizeof(clnt_addr);
                int clnt_sock = accept(fds[i].data.fd, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
                // 添加新连接到 epoll 例程
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clnt_sock;
                set_nonblock(fds[i].data.fd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
            }else {
                char msg[BUF_SIZE];
                while (1) {
                    int msg_sz = read(fds[i].data.fd, msg, BUF_SIZE);
                    if (msg_sz == 0) {
                        close(fds[i].data.fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fds[i].data.fd, NULL);
                        cout << "disconnect from client : " << fds[i].data.fd << endl;
                        break;
                    }else if (msg_sz < 0) {
                        if (errno == EAGAIN) {
                            break;
                        }
                    }else { 
                        write(fds[i].data.fd, msg, msg_sz);
                    }
                }
            }
        }
    }
    free(fds);
    close(serv_sock);
    return 0;
}

void set_nonblock(int sockfd) {
    int flag = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);
}

void error_handling(string& msg) {
    cout << msg << endl;
    exit(1);
}