#include "log.h"
#include "kit/sock.h"
#include "kit/thread_pool.h"
#include <assert.h>
#include <netinet/in.h>
#include <vector>
#include <sys/select.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

using namespace fish;

class SubThread {
  private:
    bool &running_;
    fd_set client_set_;
    int max_sock_;
    std::vector<int> sockfds;
    char buff__[1024];
    struct timeval tv__;
    pthread_mutex_t mutex_;
    bool isterminal = false;
    int id_;
  public:
    SubThread(bool &running, int id):
      running_(running),
      client_set_(),
      max_sock_(0),
      buff__(),
      tv__(),
      mutex_(),
      id_(id) {}

    ~SubThread() {
      terminal();
    }

    void push(int fd) {
      LOGD("thread[%d]: socket[%d]加入列表", id_, fd);
      lock_();
      sockfds.push_back(fd);
      unlock_();
      LOGD("thread[%d]: socket列表数量: %ld", id_, sockfds.size());
    }

    void terminal() {
      if(isterminal) return;
      isterminal = true;

      std::vector<int>::iterator it = sockfds.begin();
      lock_();
      for(;it != sockfds.end();){
        LOGD("thread[%d] socket[%d]移除监听", id_, *it);
        close(*it);
        it = sockfds.erase(it);
      }
      unlock_();
    }
    
    void register_fd_set() {
      FD_ZERO(&client_set_);
      lock_();
      std::vector<int>::iterator it = sockfds.begin();
      for(;it != sockfds.end(); it++){
        LOGD("thread[%d]: socket[%d]加入监听", id_, *it);
        if(max_sock_ < *it) max_sock_ = *it;
        FD_SET(*it, &client_set_);
      }
      unlock_();
    }

    void run() {
      LOGD("正在启动线程thread[%d]", id_);
      while(running_){
        // if(sockfds.empty()) continue;

        tv__.tv_sec = 3;
        tv__.tv_usec = 0;

        register_fd_set();
        int ret = select(max_sock_ + 1, &client_set_, NULL, NULL, &tv__);
        LOGD("sub thread ret: %d", ret);
        assert(ret != -1);
        if(ret == 0) continue;

        // 挨个确认是否有事
        std::vector<int>::iterator client_fds_it;
        lock_();
        for(client_fds_it = sockfds.begin(); client_fds_it != sockfds.end();) {
          LOGD("检查socket[%d]是否有事", *client_fds_it);
          if(FD_ISSET(*client_fds_it, &client_set_)) {
            LOGD("socket[%d]有事，正在处理...", *client_fds_it);
            ret = recv(*client_fds_it, buff__, 1024, 0);
            if(ret <= 0) {
              LOGD("scoket[%d]他说没事了，我让他滚 ----> ret = %d, %s", *client_fds_it, 
                ret, strerror(errno));
              close(*client_fds_it);
              client_fds_it = sockfds.erase(client_fds_it);
              continue;
            }
            LOGD("socket[%d]发来消息(ret: %d): %s", ret, *client_fds_it, buff__);
            if(strstr(buff__, "stop__")) {
              LOGD("程序正在退出");
              running_ = false;
            }
            bzero(buff__, sizeof(buff__));
            FD_CLR(*client_fds_it, &client_set_);
          }
          client_fds_it++;
        }
        unlock_();
      }
      terminal();
    }

  private:
    inline void lock_() {
      pthread_mutex_lock(&mutex_);
    }
    inline void unlock_() {
      pthread_mutex_unlock(&mutex_);
    }
};

int main(int argc, char **argv) {
  int port = 9000;
  if(argc == 2) port = atoi(argv[1]);
  LOGI("即将监听%d端口...", port);
  int sockfd = fish::create_socket_tcp_default(port, SOCKET_TYPE_RECV);
  int ret = listen(sockfd, 5);
  assert(ret != -1);

  LOGD("开始监听socket[%d]...", sockfd);

  fd_set sockfd_set;
  struct timeval tv;
  bool running_ = true;

  // 构造线程池
  int max_thread_max = 1;
  int cur = 0;
  fish::ThreadPool thread_pool(max_thread_max);
  SubThread *subThread[max_thread_max];
  for(int i = 0; i < max_thread_max; i++) {
    subThread[i] = new SubThread(running_, i);
    thread_pool.addLast(std::bind(&SubThread::run, subThread[i]));
  }

  while(running_) {
    FD_ZERO(&sockfd_set);
    FD_SET(sockfd, &sockfd_set);
    tv.tv_sec = 3; // 3s
    tv.tv_usec = 0;

    ret = select(sockfd + 1, &sockfd_set, NULL, NULL, &tv);
    // LOGD("ret: %d", ret);
    assert(ret != -1);
    if(ret == 0 || !FD_ISSET(sockfd, &sockfd_set)) continue;

    // 接收连接请求
    struct sockaddr_in client_addr;
    uint32_t size = (uint32_t)(sizeof(client_addr));
    int client_sockfd = accept(sockfd, (struct sockaddr*)(&client_addr), &size);
    assert(client_sockfd >= 0);

    // 分配给线程吃
    LOGD("将connfd[%d]分配给sub thread[%d]", client_sockfd, cur);
    subThread[cur++]->push(client_sockfd);
    if(cur >= max_thread_max) cur = 0;

    LOGD("socket[%d]，ip: %s, port: %d 加入了连接", client_sockfd, 
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  }

  close(sockfd);
  return 0;
}
