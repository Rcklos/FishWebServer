#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "kit/event/thread.h"
#include <pthread.h>
#include <queue>
#include <functional>

namespace fish {

typedef std::function<void()> Task;

class ThreadPool {
  typedef Task th_func_t;
  typedef int th_size_t;

private:
  ThreadPool*self;
  th_size_t max_size_;
  pthread_t *threads_;

public:
  ThreadPool();
  explicit ThreadPool(th_size_t max_size = 0);
  ~ThreadPool();
  /**
   * @breif:
   */
  void addLast(const th_func_t& func);
  th_size_t size();
  void terminal();

private:
  bool stop_;
  pthread_mutex_t mutex_;
  pthread_cond_t cond_;
  std::queue<th_func_t> mq;
  int size_;
  static void* th_create(void *args);

  th_func_t recv_task();
};

}

#endif /** __THREAD_POOL_H__ **/
