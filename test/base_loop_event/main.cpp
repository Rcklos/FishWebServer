#include "kit/event/base_loop_event.h"
#include <functional>
#include <pthread.h>
#include <unistd.h>
#include <thread>
#include "kit/thread_pool.h"
#include "log.h"

class TestTask: public fish::BaseLoopEventGroup {
private:
  char *name;
  pthread_mutex_t mutex_;
  int count_ = 0;
public:
  TestTask(const char *name): name(const_cast<char *>(name)),
    fish::BaseLoopEventGroup(1) {}

  void setup_() {
    pthread_mutex_init(&mutex_, NULL);
    LOGD("thread[%ld]: hello %s", pthread_self(), name);
  }

  void loop_() {
    if(count_ ==  pool_size_) terminal();
  }

  void run_() {
    increase();
    LOGD("thread[%ld]: run %d", pthread_self(), count_);
  }

  bool isTerminal() {
    return count_ >= pool_size_;
  }
private:
  void increase() {
    int lock = pthread_mutex_lock(&mutex_);
    LOGD("thread[%ld] lock = %d", pthread_self(), lock);
    count_++;
    pthread_mutex_unlock(&mutex_);
  }
};

int main() {
  // std::string name = "NO.1";
  // TestHanlde handle = name;
  // SubThread sub_thread(std::bind(&TestHanlde::handle, &handle));
  // sub_thread.loop(std::bind(testHanlde));
  
  // int size = 5;
  // fish::ThreadPool thread_pool(size);
  // TestTask task = "test";
  // const fish::Task t = (std::bind(&TestTask::run_, &task));
  // const fish::Task *pt = &t;
  // for(int i = 0; i < size; i++) {
  //   thread_pool.addLast(t);
  // }

  TestTask task = "main";
  task.loop(std::bind(&TestTask::run_, &task));
  
  while(!task.isTerminal());
  return 0;
}
