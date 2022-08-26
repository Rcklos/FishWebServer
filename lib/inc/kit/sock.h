#ifndef __UTIL_SOCK_H__
#define __UTIL_SOCK_H__

#include <string.h>
#include <arpa/inet.h>

#define DEFAULT_SOCK_PORT 9000

namespace fish{

typedef int socket_t;

/**
 * @addr 目前是sockaddr_in
 */
typedef struct socket_addr_t {
  const char *ip;
  int port;
  void *sockaddr;

  socket_addr_t(char *ip, int port):
    ip(ip),
    port(port) {
      if(!ip && port < 0) {
        this->sockaddr = nullptr;
        return;
      }
      struct sockaddr_in *sockaddr = new struct sockaddr_in();
      memset(sockaddr, 0, sizeof(struct sockaddr_in));
      sockaddr->sin_family = AF_INET;
      sockaddr->sin_port = htons(port);
      sockaddr->sin_addr.s_addr = ip? inet_addr(ip): INADDR_ANY;

      this->sockaddr = sockaddr;
    }

  ~socket_addr_t() {
    if(sockaddr)
      delete (struct sockaddr_in*)sockaddr;
  }
} socket_addr_t;

typedef enum {
  SOCKET_TYPE_RECV,
  SOCKET_TYPE_SEND
} socket_type_t;

/**
 * @deprecated
 * @brief 默认值初始化socket
 * @param port 端口
 * @param type 类型
 * @return 成功返回sockfd, 失败返回-1
 */
int create_socket_tcp_default(int port, socket_type_t type);

/**
 * @brief 默认值初始化socket
 * @param type 类型
 * @param port 端口
 * @return 成功返回sockfd, 失败返回-1
 */
socket_t create_socket_tcp_default(socket_type_t type, int port = -1);

inline int connect(socket_t sockfd, socket_addr_t *addr) {
  return connect(sockfd, (struct sockaddr*)addr->sockaddr, sizeof(struct sockaddr_in));
}

}
#endif /** __UTIL_SOCK_H__ **/
