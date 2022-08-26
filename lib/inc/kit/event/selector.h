#ifndef __EVENT_SELECTOR_H__
#define __EVENT_SELECTOR_H__
#include "kit/event/event_listener.h"
#include "kit/thread_pool.h"
#include "kit/event/base_task_group.h"
#include "kit/sock.h"
#include <pthread.h>
#include <queue>
#include <vector>
namespace fish{

typedef struct select_task_buff_t {
  char buff__[1024];
  std::queue<socket_t> mq;
  bool *running_;
  pthread_mutex_t mq_mutex_;

  select_task_buff_t() {
    pthread_mutex_init(&mq_mutex_, NULL);
  }

  ~select_task_buff_t() {
    pthread_mutex_destroy(&mq_mutex_);
  }
} select_task_buff_t;

class SelectTask {
private:
  // char buff_[1024];
public:
  bool &running_;
  std::queue<socket_t> mq_;
  pthread_mutex_t mq_mutex_;

public:
  void run(EventListener *listener);
  SelectTask(bool &running_);
  ~SelectTask();
};

class Selector: protected BaseTaskGroup {
private:
  socket_t sockfd_;
  int dispatch_cur;
// basetaskgroup
private:
  void before_register();
  void after_register();
//
private:
  // select_task_buff_t *select_task_buff_;
  SelectTask **select_tasks;
  // pthread_mutex_t mutex_;
  void dispatch(socket_t client);
public:
  Selector(socket_t sockfd, int pool_size);
  ~Selector();
  bool is_terminal();
  // void bootstrap(const Task& task);
  void bootstrap(EventListener *listener);
};

}
#endif /** __EVENT_SELECTOR_H__ **/
