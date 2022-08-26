#ifndef __KIT_EVENT_LISTENER__
#define __KIT_EVENT_LISTENER__
#include "kit/event/base_task_group.h"
#include "kit/sock.h"
#include "kit/thread_pool.h"
#include <functional>
namespace fish {

typedef enum {
  EVENT_TYPE_NOP,
  EVENT_TYPE_RECV_SOCK,
} event_type_t;


typedef struct event_t {
  socket_t sockfd;
  event_type_t type;
  char *buff;
  int buff_size;

  event_t():buff_size(0) {}
  ~event_t() {
    if(buff) delete[] buff;
  }
} event_t;

typedef std::function<void(event_t *)> ListenFunc_t;

class EventListener {
  private:
    // 线程池
    ThreadPool *tpool;
  // protected:
  public:
    virtual void listen(event_t *event);
    EventListener();
    ~EventListener();
    void emit(event_t *event);
};

}
#endif /* __KIT_EVENT_LISTENER__ */
