#include "kit/sock.h"
#include "log.h"
#include <assert.h>
#include <cerrno>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char **argv) {
  const char *ip = "127.0.0.1";
  int port = 9000;
  if(argc >=  2) port = atoi(argv[2]);
  if(argc >=  3) ip = argv[1];

  fish::socket_t sockfd = fish::create_socket_tcp_default(fish::SOCKET_TYPE_SEND);
  fish::socket_addr_t *addr = new fish::socket_addr_t(const_cast<char *>(ip), port);

  LOGD("连接服务器[%s:%d]...", ip, port);
  int ret = fish::connect(sockfd, addr);
  LOGD("sockefd[%d] -------> %s", sockfd, strerror(errno));
  assert(ret != -1);

  std::string str;
  while(1) {
    printf("输入消息: ");
    std::cin >> str;
    LOGD("发送消息: %s", str.c_str());
    send(sockfd, str.c_str(), str.length(), 0);

    if(strstr(str.c_str(), "stop__")) break;
  }
  LOGD("bye");
  delete addr;
  close(sockfd);
}
