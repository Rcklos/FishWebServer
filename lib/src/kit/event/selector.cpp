#include "kit/event/selector.h"
#include "kit/event/event_listener.h"
#include "kit/sock.h"
#include "kit/thread_pool.h"
#include "log.h"
// #include <fcntl.h>
#include <cerrno>
#include <cstddef>
#include <assert.h>
#include <functional>
#include <pthread.h>
#include <strings.h>
#include <unistd.h>
#include <sys/select.h>

using namespace fish;

Selector::Selector(socket_t sockfd, int pool_size):
  sockfd_(sockfd), dispatch_cur(0), BaseTaskGroup(pool_size) {
  // select_task_buff_ = new select_task_buff_t[pool_size];
  select_tasks = new SelectTask*[pool_size_];
  for(int i = 0; i < pool_size_; i++)
    select_tasks[i] = new SelectTask(running_);
}

Selector::~Selector() {
  // delete[] select_task_buff_;
  for(int i = 0; i < pool_size_; i++)
    delete select_tasks[i];
  delete [] select_tasks;
}

void Selector::before_register() {}

void Selector::after_register() {
  fd_set sockfd_set;
  struct timeval tv;
  int ret = -1;
  FD_ZERO(&sockfd_set);
  FD_SET(sockfd_, &sockfd_set);

  LOGD("select reactor正在监听");
  while(running_) {
    FD_ZERO(&sockfd_set);
    FD_SET(sockfd_, &sockfd_set);
    tv.tv_sec = 30;
    tv.tv_usec = 0;

    ret = select(sockfd_ + 1, &sockfd_set, NULL, NULL, &tv);
    assert(ret != -1);

    if(ret ==  0 || !FD_ISSET(sockfd_, &sockfd_set)) continue;
    struct sockaddr_in client_addr;
    uint32_t size = (uint32_t)(sizeof(client_addr));
    int client_sockfd = accept(sockfd_, (struct sockaddr*)(&client_addr), &size);
    assert(client_sockfd >= 0);
    dispatch(client_sockfd);
    LOGD("socket[%d]，ip: %s, port: %d 加入了连接", client_sockfd, 
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  }
}

void Selector::dispatch(socket_t client) {
  // select_task_buff_t buff = select_task_buff_[dispatch_cur++];
  // *buff.running_ = false;
  
  SelectTask *select_task = select_tasks[dispatch_cur++];
  pthread_mutex_lock(&select_task->mq_mutex_);
  LOGD("正在将client[%d]分配给线程 dispatch_cur: %d", client, dispatch_cur);
  select_task->mq_.push(client);
  pthread_mutex_unlock(&select_task->mq_mutex_);
  if(dispatch_cur >= pool_size_) dispatch_cur = 0;
}

SelectTask::SelectTask(bool &running): running_(running) {}
SelectTask::~SelectTask(){}

void SelectTask::run(EventListener *listener) {
  pthread_t tid = pthread_self();
  fd_set sock_set;
  struct timeval tv;
  int size = 0, max = 0, ret = 0;
  socket_t sock;
  std::queue<socket_t> q;
  LOGD("正在启动select线程thread[%ld]", tid);
  // bzero(buff_, 1024);
  while(running_) {
    FD_ZERO(&sock_set);
    // 先读取mq长度
    pthread_mutex_lock(&mq_mutex_);
    size = mq_.size();
    // LOGD("thread[%ld]: 开始读取mq, size = %d", tid, size);
    for(int i = 0; i < size; i++) {
      // select
      sock = mq_.front();
      mq_.pop();
      q.push(sock);
      // LOGD("thread[%ld]: 将connfd[%d]加入了监听队列", tid, sock);
    }
    pthread_mutex_unlock(&mq_mutex_);

    size = q.size();
    for(int i = 0; i < size; i++) {
      sock = q.front();
      q.pop();
      if(max < sock) max = sock;
      // LOGD("thread[%ld]: 将connfd[%d]加入了select队列", tid, sock);
      FD_SET(sock, &sock_set);
      q.push(sock);
    }

    tv.tv_sec = 3;
    tv.tv_usec = 0;
    ret = select(max + 1, &sock_set, NULL, NULL, &tv);
    assert(ret != -1);
    size = q.size();
    for(int i = 0; i < size; i++) {
      sock = q.front();
      q.pop();
      if(FD_ISSET(sock, &sock_set)) {
        event_t *event = new event_t();
        event->sockfd = sock;
        event->buff = new char[1024];
        char *buff_ = event->buff;
        LOGD("socket[%d]有事，正在处理...", sock);
        ret = recv(sock, buff_, 1024, 0);
        if(ret <= 0) {
          LOGD("scoket[%d]他说没事了，我让他滚 ----> ret = %d, %s", sock, ret, strerror(errno));
          close(sock);
          continue;
        }
        // LOGD("socket[%d]发来消息(ret: %d): %s", ret, sock, buff_);
        event->buff_size = ret;
        event->type = EVENT_TYPE_RECV_SOCK;
        listener->emit(event);
        // bzero(buff_, 1024);
        FD_CLR(sock, &sock_set);
      }
      // 入队下次用
      q.push(sock);
    }
  }
  LOGD("thread[%ld]: 正在退出线程", tid);
}

bool Selector::is_terminal() {
  return running_ == false;
}

void Selector::bootstrap(EventListener *listener) {
  for(int i = 0; i < pool_size_; i++) {
    // select_task_buff_[i].running_ = &running_;
    // register_task(std::bind(select_task, select_task_buff_ + i));
    register_task(std::bind(&SelectTask::run, select_tasks[i], listener));
  }
}
