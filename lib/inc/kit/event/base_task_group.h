#ifndef __BASE_TASK_GROUP_H__
#define __BASE_TASK_GROUP_H__

#include "kit/thread_pool.h"
#include "kit/sock.h"
namespace fish {

class BaseTaskGroup {
protected:
  socket_t sockfd_;
  int pool_size_;
  bool running_;

  virtual void before_register();
  virtual void after_register();
  int register_task(const Task &thread);

public:
  BaseTaskGroup(int pool_size = 0);
  ~BaseTaskGroup();
  void terminal();

private:
  ThreadPool *thread_pool_;
};

}
#endif /* __BASE_TASK_GROUP_H__ */
