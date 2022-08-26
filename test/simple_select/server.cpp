#include "log.h"
#include "kit/sock.h"
#include <assert.h>
#include <cerrno>
#include <netinet/in.h>
#include <vector>
#include <sys/select.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

using namespace fish;

int main(int argc, char **argv) {
  int port = 9000;
  if(argc == 2) port = atoi(argv[1]);
  LOGI("即将监听%d端口...", port);
  int sockfd = fish::create_socket_tcp_default(port, SOCKET_TYPE_RECV);
  int ret = listen(sockfd, 5);
  assert(ret != -1);

  LOGD("开始监听socket[%d]...", sockfd);

  // struct sockaddr_in client;
  // socklen_t client_size = sizeof(client);
  // int connfd = accept(sockfd, (struct sockaddr*)&client, &client_size);
  // return 0;

  std::vector<int> client_fds;
  std::vector<int>::iterator client_fds_it = client_fds.begin();
  fd_set client_set;
  char buff__[1024];
  struct timeval tv;
  int maxsock = sockfd;
  bool running_ = true;
  while(running_) {
    FD_ZERO(&client_set);
    // LOGD("server socket[%d]加入监听列表", sockfd);
    FD_SET(sockfd, &client_set);
    tv.tv_sec = 30; // 30s
    tv.tv_usec = 0;

    // 加入监听
    LOGD("监听fd数量: %d", (int)client_fds.size());
    for(client_fds_it = client_fds.begin(); client_fds_it != client_fds.end();) {
      if(*client_fds_it == 0) {
        LOGD("socket[%d]断开了连接, 不再监听", *client_fds_it);
        client_fds_it = client_fds.erase(client_fds_it);
        continue;
      }
      // LOGD("socket[%d]加入监听", *client_fds_it);
      FD_SET(*client_fds_it, &client_set);
      client_fds_it++;
    }

    ret = select(maxsock + 1, &client_set, NULL, NULL, &tv);
    // LOGD("ret: %d", ret);
    assert(ret != -1);
    if(ret == 0) continue;
    
    for(client_fds_it = client_fds.begin(); client_fds_it != client_fds.end();) {
      LOGD("检查socket[%d]是否有事", *client_fds_it);
      // 挨个确认是否有事
      if(*client_fds_it ==  0) {
        LOGD("socket[%d]断开了连接", *client_fds_it);
        client_fds_it = client_fds.erase(client_fds_it);
        continue;
      }
      if(FD_ISSET(*client_fds_it, &client_set)) {
        LOGD("socket[%d]说他有事，正在处理...", *client_fds_it);
        ret = recv(*client_fds_it, buff__, 1024, 0);
        if(ret <= 0) {
          LOGD("scoket[%d]他说没事了，我让他滚 ----> ret = %d, %s", *client_fds_it, 
              ret, strerror(errno));
          close(*client_fds_it);
          FD_CLR(*client_fds_it, &client_set);
          client_fds_it = client_fds.erase(client_fds_it);
          continue;
        }
        LOGD("socket[%d]发来消息(ret: %d): %s", ret, *client_fds_it, buff__);
        if(strstr(buff__, "stop__")) {
          LOGD("程序正在退出");
          running_ = false;
        }
        bzero(buff__, sizeof(buff__));
        FD_CLR(*client_fds_it, &client_set);
      }
      client_fds_it++;
    }

    // 接收连接请求
    if(FD_ISSET(sockfd, &client_set)) {
      struct sockaddr_in client_addr;
      uint32_t size = (uint32_t)(sizeof(client_addr));
      int client_sockfd = accept(sockfd, (struct sockaddr*)(&client_addr), &size);
      assert(client_sockfd >= 0);
      if(maxsock < client_sockfd) maxsock = client_sockfd;
      client_fds.push_back(client_sockfd);
      strcpy(buff__, "hello my friend\n");
      send(client_sockfd, buff__, strlen(buff__), 0);
      bzero(buff__, sizeof(buff__));
      ret = recv(client_sockfd, buff__, 1024, 0);
      LOGI("接收到socket[%d]消息： %s", client_sockfd, buff__);
      bzero(buff__, sizeof(buff__));
      LOGD("socket[%d]，ip: %s, port: %d 加入了连接", client_sockfd, 
          inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
  }

  for(client_fds_it = client_fds.begin(); client_fds_it != client_fds.end();)  {
    close(*client_fds_it);
    client_fds_it = client_fds.erase(client_fds_it);
  }

  close(sockfd);

  return 0;
}
