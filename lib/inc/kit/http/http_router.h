#ifndef __HTTP_ROUTER_H__
#define __HTTP_ROUTER_H__

#include "kit/http/http_request.h"
#include "kit/http/http_response.h"
#include <functional>
#include <string>
namespace fish {

typedef std::function<void(HttpRequest*, HttpResponse*)> RouteTask;

class HttpRouter {
private:
  HttpRouter* root;
  HttpRouter* prev;
  HttpRouter* next;
  HttpRouter* parent;
  HttpRouter* child;
  std::string pattern;
  RouteTask task;
  HttpRouter(HttpRouter *root, std::string pattern, const RouteTask &task);
  static bool route_(HttpRouter *parent, std::string pattern, const RouteTask &task);
  static bool dispatch_(HttpRouter* router, std::string pattern, HttpRequest* req, HttpResponse* res);
public:
  ~HttpRouter();
  static HttpRouter* createRoot(const RouteTask &task);
  bool route(std::string pattern, const RouteTask &task);
  bool dispatch(HttpRequest *req, HttpResponse *res);
  bool remove();
};

}
#endif /* __HTTP_ROUTER_H__ */
