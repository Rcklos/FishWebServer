#include "kit/event/base_server.h"
#include "kit/event/event_listener.h"
#include <functional>
#include "kit/http/http_handle.h"
#include "kit/http/http_request.h"
#include "kit/http/http_response.h"
#include "log.h"

using namespace fish;

class MyEventListener: public EventListener {
  private:
    HttpHandle http_handle;
  public:
    virtual void listen(event_t *event){ 
      // LOGD("收到了event的消息: %s", event->buff);
      http_handle.handle(event->sockfd, event->buff, event->buff_size);
      if(event) delete event;
    }
  HttpHandle& getHandle() {
    return http_handle;
  }
};

int main() {
  BaseServer server;
  MyEventListener *listener = new MyEventListener();
  listener->getHandle().route("/", [](HttpRequest *req, HttpResponse *res){
      res->write("你好啊！！！\n");
  });
  listener->getHandle().route("/test", [](HttpRequest *req, HttpResponse *res){
      res->write("这是个test\n");
  });
  listener->getHandle().route("/test/test", [](HttpRequest *req, HttpResponse *res){
      res->write("这是嵌套的test\n");
  });
  listener->getHandle().route("/bad", [](HttpRequest *req, HttpResponse *res){
      res->write(SC_BAD_REQUEST);
      res->end();
  });
  server.bind(9000)
    ->wait_for_accept(fish::SERVER_ACCEPT_SELECT, listener);
  delete listener;
  return 0;
}
