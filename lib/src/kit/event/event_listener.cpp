#include "kit/event/event_listener.h"
#include <cstddef>
#include "log.h"
#include <functional>
#include <pthread.h>

using namespace fish;

EventListener::EventListener() {}

EventListener::~EventListener() {
  if(tpool != NULL) delete tpool;
  tpool = NULL;
}

void EventListener::listen(event_t *event) {
  LOGD("default event listener");
}

void hello(event_t *e) {
  LOGD("hello: %s", e->buff);
  delete e;
}

void EventListener::emit(event_t *event) {
  if(tpool ==  NULL) {
    tpool = new ThreadPool(5);
    LOGD("create listener thread pool");
  }
  LOGD("emit event");
  tpool = new ThreadPool(5);
  // tpool->addLast(std::bind(hello, event));
  tpool->addLast(std::bind(&EventListener::listen, this, event));
}


