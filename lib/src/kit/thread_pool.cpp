#include "kit/thread_pool.h"
#include <assert.h>
#include "log.h"
#include <functional>
#include <unistd.h>

using namespace fish;

ThreadPool::ThreadPool(th_size_t max_size):
  stop_(false),
  max_size_(max_size) {
  pthread_mutex_init(&this->mutex_, NULL);
  pthread_cond_init(&this->cond_, NULL);

  // 创建线程池
  // this->threads_ = (pthread_t *)malloc(sizeof(pthread_t) * this->max_size_);
  this->threads_ = new pthread_t[max_size];
  for(int i = 0; i < max_size; i++)
    pthread_create(&this->threads_[i], NULL, th_create, this);

  // 指针
  this->self = this;
}

// 接受线程任务
ThreadPool::th_func_t ThreadPool::recv_task() {
  ThreadPool::th_func_t func = NULL;
  pthread_mutex_lock(&this->mutex_);
  // 阻塞等待
  while(this->mq.empty() && !this->stop_) 
    pthread_cond_wait(&this->cond_, &this->mutex_);

  if(this->stop_) { 
    pthread_mutex_unlock(&this->mutex_);
    return NULL;
  }

  assert(!this->mq.empty());
  func = this->mq.front();
  this->mq.pop();
  LOGD("取出任务执行 size: %ld", this->mq.size());
  size_--;
  pthread_mutex_unlock(&this->mutex_);
  return func;
}

ThreadPool::~ThreadPool() {
  this->terminal();
}

void ThreadPool::terminal() {
  LOGD("thread pool: terminal");
  if(this->stop_) return;
  this->stop_ = true;
  pthread_cond_broadcast(&this->cond_);
  for(int i = 0; i < this->max_size_; i++) 
    pthread_join(this->threads_[i], NULL);

  // 回收资源
  // free(threads_);
  delete[] threads_;
  pthread_mutex_destroy(&this->mutex_);
  pthread_cond_destroy(&this->cond_);
}

void* ThreadPool::th_create(void *args) {
  // 获取自身的对象
  pthread_t tid = pthread_self();
  ThreadPool *pool = (ThreadPool *)args;

  // 取出任务并执行
  while(!pool->stop_) {
    ThreadPool::th_func_t func = pool->recv_task();
    // 此时认为已经退出了
    if(func ==  NULL) {
      LOGD("thread pool: receive null ptr --> stop: %d", pool->stop_);
      break;
    }
    func();
    LOGD("thread[%ld] 执行完毕");
  }
  LOGD("thread: bye ---> stop: %d", pool->stop_);
  return 0;
}

ThreadPool::th_size_t ThreadPool::size() {
  return size_;
}

void ThreadPool::addLast(const th_func_t& func) {
  pthread_mutex_lock(&this->mutex_);
  this->mq.push(func);
  LOGD("ThreadPool: 有线程任务进入队列 size = %ld", this->mq.size());
  size_++;
  pthread_mutex_unlock(&this->mutex_);
  pthread_cond_signal(&this->cond_);
}

// void ThreadPool::addLast(Runnable &task) {
//   pthread_mutex_lock(&this->mutex_);
//   this->mq.push(std::bind(&Runnable::run, task));
//   LOGD("ThreadPool: 有线程任务进入队列 size = %ld", this->mq.size());
//   pthread_mutex_unlock(&this->mutex_);
//   pthread_cond_signal(&this->cond_);
// }
