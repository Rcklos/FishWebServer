#include "kit/event/base_task_group.h"
#include "kit/thread_pool.h"
#include "log.h"
#include <functional>

using namespace fish;

void BaseTaskGroup::before_register() {}
void BaseTaskGroup::after_register() {}

BaseTaskGroup::BaseTaskGroup(int pool_size):
  sockfd_(0),
  pool_size_(pool_size),
  running_(true),
  thread_pool_(nullptr){}

BaseTaskGroup::~BaseTaskGroup() {
  this->terminal();
}

void BaseTaskGroup::terminal() {
  if(!running_) return;
  LOGD("BaseTaskGroup: terminal");
  running_ = false;
  if(thread_pool_ != NULL) delete thread_pool_;
}


int BaseTaskGroup::register_task(const Task &thread) {
  static int count = 0;
  if(thread_pool_ == NULL) {
    LOGD("pool size: %d", pool_size_);
    before_register();
    thread_pool_ = new ThreadPool(pool_size_);
  }
  if(count < pool_size_) {
    LOGD("register task, count = %d", count);
    thread_pool_->addLast(thread);
    count++;
    LOGD("服务注册成功! 线程池长度: %d", pool_size_);
  }
  if(count ==  pool_size_){
    LOGD("BaseTaskGroup: running_ = %d", running_);
    after_register();
    LOGD("BaseTaskGroup: bye");
    count++;
  }
  return 0;
}
