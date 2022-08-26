#include "kit/sock.h"
#include "log.h"
#include <assert.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace fish;

int fish::create_socket_tcp_default(int port, socket_type_t type) {
  // 这里应该是client，暂不实现
  if(port ==  -1) port = 9000;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int reuse = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  if(type == fish::SOCKET_TYPE_RECV) {
    LOGD("绑定端口: %d", port);
    int ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret != -1);
  }
  return sock;
}

fish::socket_t fish::create_socket_tcp_default(socket_type_t type, int port) {
  fish::socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  if(type == fish::SOCKET_TYPE_SEND) return sock;

  if(port ==  -1) port = DEFAULT_SOCK_PORT;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  int reuse = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  if(type == fish::SOCKET_TYPE_RECV) {
    LOGD("绑定端口: %d", port);
    int ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret != -1);
  }
  return sock;
}

