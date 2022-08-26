#ifndef __HTTP_HANDLE_H__
#define __HTTP_HANDLE_H__
#include "kit/http/http_router.h"
#include "kit/sock.h"
#include "kit/thread_pool.h"
namespace fish {

class HttpHandle {
  private:
    ThreadPool *tpool_;
    void handle_(socket_t sockfd, char *buff_, int size);
    HttpRouter *router;
  public:
    HttpHandle();
    ~HttpHandle();
    bool route(std::string pattern, const RouteTask &task);
    void handle(socket_t sockfd, char *buff_, int size);
};

}
#endif /* __HTTP_HANDLE_H__ */
