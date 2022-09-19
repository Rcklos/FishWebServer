/*================================================================*
        Copyright (C) 2021 All rights reserved, www.lentme.cn.
      	文件名称：test_coroutine.cc
      	创 建 者：fish
      	创建日期：2022年9月18日
 *================================================================*/

#include "common.h"
#include "coroutine.h"
#include <cerrno>
#include <cstring>
#include <functional>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>

#include <queue>
#include <vector>

#define IPADDRESS "127.0.0.1"
#define PORT 6666
#define MAXSIZE 1024
#define LISTENQ 5
#define FDSIZE 1000
#define EPOLLEVENTS 100
#define SOCKET_BIND() socket_bind(IPADDRESS, PORT)


int socket_bind(const char* ip, int port);
void do_epoll(int listenfd);
void handle_events(int epollfd, struct epoll_event* events, int num, int listenfd, char* buf);
void handle_accept(int epollfd, int listenfd);
void do_read(int epollfd, int fd, char* buf);
void do_write(int epollfd, int fd, char* buf);
void add_event(int epollfd, int fd, int state);
void modify_event(int epollfd, int fd, int state);
void delete_event(int epollfd, int fd, int state);

// epoll
void epoll_test(int process_id, int co_id) {
    int sockfd = socket_bind(IPADDRESS, PORT);
    listen(sockfd, LISTENQ);
    do_epoll(sockfd);
}

// coroutine
struct Client {
    int connfd = -1;
    Client(int connfd): connfd(connfd) {}
};

struct HandleTask {
    typedef std::shared_ptr<HandleTask> Ptr;
    fish::Coroutine::Ptr co = nullptr;
    std::shared_ptr<Client> &client;
    HandleTask(fish::Coroutine::Ptr coroutine, std::shared_ptr<Client> &client):
        co(coroutine), client(client) {}
};

void client_handle(int process_id, int co_id, std::shared_ptr<Client> client) {
    if(!client) return;
    char buff[MAXSIZE];
    while(recv(client->connfd, buff, MAXSIZE, 0) > 0) {
        std::string buffer(buff);
        FISH_LOGDEBUG(
            "process: " << process_id
            << "co_id: " << co_id
            << "recv frome client[" << client->connfd << "]"
            << ": " << buffer);
    }
}

void coroutine_handle(std::queue<HandleTask::Ptr> &task_queue, int process_id, int co_id, 
        std::shared_ptr<Client> client) {
    while(1) {
        while(!client) {
            task_queue.push(std::make_shared<HandleTask>(fish::Coroutine::GetCurCoroutine(), client));
            fish::Coroutine::Yield();
        }
        client_handle(process_id, co_id, client);
        close(client->connfd);
        client = nullptr;
        FISH_LOGDEBUG(
            "process: " << process_id
            << "co_id: " << co_id
            << "close connect with client[" << client->connfd << "]");
    }
}
void coroutine_test(std::queue<HandleTask::Ptr> &task_queue, int process_id, int co_id) {
    FISH_LOGDEBUG(
        "process: " << process_id
        << "co_id: " << co_id
        << "starting server in coroutine way");

    int listenfd = SOCKET_BIND();
    int ret = listen(listenfd, 5);
    FISH_LOGDEBUG(
            "process: " << process_id
            << "co_id: " << co_id
            << "listening...");
    if(ret == -1) {
        FISH_LOGERROR(
            "process: " << process_id
            << "co_id: " << co_id
            << "server listen failed, " << std::strerror(errno));
        exit(1);
    }

    struct sockaddr_in client;
    socklen_t client_addr_len = sizeof(client);
    while(1) {
        int connfd = accept(listenfd, (struct sockaddr*)&client, &client_addr_len);
        if(connfd == -1) {
            FISH_LOGERROR(
                "process: " << process_id
                << "co_id: " << co_id
                << "connect cli error, " << std::strerror(errno));
            continue;
        }
        char remote[INET_ADDRSTRLEN];
        FISH_LOGDEBUG(
            "process: " << process_id
            << "co_id: " << co_id
            << "conn with ip: "
            << inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN)
            << " port: "
            << ntohs(client.sin_port));
        auto client = std::make_shared<Client>(connfd);
        auto handle = task_queue.front();
        task_queue.pop();
        handle->client = client;
        handle->co->Resume();
    }
    close(listenfd);
}

void server_test(int process_id) {
    std::queue<HandleTask::Ptr> task_queue;
    std::vector<fish::Coroutine::Ptr> v;
    fish::Coroutine::StackMem share_mem;
    auto co_acceptor = fish::Coroutine::Create(
            std::bind(coroutine_test, task_queue, process_id, 0), &share_mem);
    for(int i = 1; i < 100; i++) {
        auto co_handler = fish::Coroutine::Create(
                std::bind(coroutine_handle, task_queue, process_id, i, nullptr), &share_mem);
        v.push_back(co_handler);
    }
    for(auto x: v) x->Resume();
    while(1) {
        co_acceptor->Resume();
    }
}

void boostrap() {
    server_test(getpid());
}

int main (int argc, char *argv[]) {
    boostrap();
    return 0;
}

int socket_bind(const char* ip, int port) {
    struct sockaddr_in addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        FISH_LOGERROR("server failed to create sockfd");
        exit(1);
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);
    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        FISH_LOGERROR("server failed to bind sockaddr");
        exit(1);
    }
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    return sockfd;
}

void do_epoll(int sockfd) {
    struct epoll_event events[EPOLLEVENTS];
    int ret;
    int epollfd;
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    epollfd = epoll_create(FDSIZE);
    add_event(epollfd, sockfd, EPOLLIN);
    while(1) {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, sockfd, buf);
    }
    close(sockfd);
}

void handle_events(int epollfd, struct epoll_event* events, int num, int listenfd, char* buf)
{
    int i, fd;
    for(i = 0; i<num; i++) {
    	fd = events[i].data.fd;
    	if((fd == listenfd) && (events[i].events & EPOLLIN))
    		handle_accept(epollfd, listenfd);
    	else if(events[i].events & EPOLLIN)
    		do_read(epollfd, fd, buf);
    	else if(events[i].events & EPOLLOUT)
    		do_write(epollfd, fd, buf);
    }
}

void handle_accept(int epollfd, int listenfd)
{
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
    if(clifd == -1) {
        FISH_LOGERROR("accept error: ");
    }
    else {
    	printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
    	add_event(epollfd, clifd, EPOLLIN);
    }
}

void do_read(int epollfd, int fd, char* buf) {
    int nread;
    nread = read(fd, buf, MAXSIZE);
    if(nread == -1) {
        FISH_LOGERROR("read error");
    	close(fd);
    	delete_event(epollfd, fd, EPOLLIN);
    }
    else if(nread == 0) {
        FISH_LOGERROR("client close");
    	close(fd);
    	delete_event(epollfd, fd, EPOLLIN);
    }
    else {
    	printf("read message is : %s", buf);
    	modify_event(epollfd, fd, EPOLLOUT);
    }
}

void do_write(int epollfd, int fd, char* buf) {
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    if(nwrite == -1) {
        FISH_LOGERROR("write error!")
    	close(fd);
    	delete_event(epollfd, fd, EPOLLIN);
    }
    else
    	modify_event(epollfd, fd, EPOLLIN);
    memset(buf, 0, MAXSIZE);
}

void add_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void delete_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void modify_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

