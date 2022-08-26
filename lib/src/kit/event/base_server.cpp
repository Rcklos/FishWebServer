#include "kit/event/base_server.h"
#include "kit/event/event_listener.h"
#include "log.h"
#include "kit/event/selector.h"
#include <functional>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>

using namespace fish;

BaseServer::BaseServer():
  ip_(nullptr),
  port_(-1),
  sockfd_(0),
  self_(this), 
  enable_thread_pool_(false) {}

BaseServer::~BaseServer() {
  close(sockfd_);
}

void IBaseHandle::handle() {}

BaseServer* BaseServer::bind(char *ip, int port) {
  ip_ = ip;
  port_ = port;
  sockfd_ = create_socket_tcp_default(SOCKET_TYPE_RECV, port);
  assert(sockfd_ > 0);
  return self_;
}

BaseServer* BaseServer::bind(int port) {
  return self_->bind(NULL, port);
}

socket_t BaseServer::sock() { return sockfd_; }

void BaseServer::wait_for_accept(server_accept_t accept, EventListener *listener) {
  int ret = listen(sockfd_, 5);
  assert(ret != -1);
  LOGD("开始监听socket[%d]...", sockfd_);
  switch(accept) {
    case SERVER_ACCEPT_SELECT:
      self_->select_accept(listener);
      break;
    case SERVER_ACCEPT_POLL:
    case SERVER_ACCEPT_EPOLL:
      break;
  }
}

void BaseServer::select_accept(EventListener *listener) {
  Selector selector(sockfd_, 1);
  selector.bootstrap(listener);
  while(!selector.is_terminal());
}
