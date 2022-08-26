#include "kit/http/http_router.h"
#include <sstream>
#include "log.h"

using namespace fish;
using namespace std;

HttpRouter::HttpRouter(HttpRouter *root, std::string pattern, const RouteTask& task):
  root(root),
  pattern(pattern),
  task(task) {}

HttpRouter::~HttpRouter() {
  remove();
}

HttpRouter* HttpRouter::createRoot(const RouteTask &task) {
  HttpRouter *root = new HttpRouter(NULL, "/", task);
  root->root = root;
  if(!root) return NULL;
  return root;
}

bool HttpRouter::route_(HttpRouter *parent, std::string pattern, const RouteTask &task) {
  if(!parent) return false;
  LOGD("create route --> parent: %s, pattern: %s", parent->pattern.c_str(), pattern.c_str());
  if(pattern[0] ==  '/') pattern.erase(0, 1);
  if(pattern.length() ==  0 || pattern ==  "/") {
    parent->root->task = task;
    return true;
  }
  if(pattern[pattern.length() - 1] ==  '/') pattern.erase(pattern.length() - 1);
  if(!parent->child)
    parent->child = new HttpRouter(parent->root, "/**", NULL);
  auto pos = pattern.find_first_of("/");
  if(pos == string::npos) {
    HttpRouter *tmp = parent->child;
    // 等价匹配
    while(tmp->next && tmp->next->pattern != pattern) tmp = tmp->next;
    if(!tmp->next)
      tmp->next = new HttpRouter(parent->root, pattern, task);
    tmp->next->prev = tmp;
    tmp = tmp->next;
    tmp->parent = parent;
    return true;
  }
  std::string parentRoute = pattern.substr(0, pos);
  auto tmp = parent->child;
  while(tmp->next && tmp->next->pattern != parentRoute) tmp = tmp->next;
  if(!tmp->next)
    tmp->next = new HttpRouter(parent->root, parentRoute, NULL);
  tmp->next->prev = tmp;
  tmp = tmp->next;
  string newPattern = pattern.substr(pos + 1, string::npos);
  return route_(tmp, newPattern, task);
}

bool HttpRouter::route(string pattern, const RouteTask &task) {
  return route_(root, pattern, task);
}

bool HttpRouter::dispatch_(HttpRouter *router, string pattern, HttpRequest *req, HttpResponse *res) {
  LOGD("match route: router: %s, child: %d, pattern: %s", router->pattern.c_str(), 
      router->child == NULL, pattern.c_str());
  if(!router->child) return false;
  auto pos = pattern.find_first_of("/");
  if(pos == string::npos) {
    auto tmp = router->child->next;
    while(tmp && tmp->pattern != pattern) tmp = tmp->next;
    if(!tmp) return false;
    tmp->task(req, res);
    return true;
  }
  string parentRoute = pattern.substr(0, pos);
  auto tmp = router->child->next;
  while(tmp && tmp->pattern != parentRoute) tmp = tmp->next;
  if(!tmp) return false;
  string newPattern = pattern.substr(pos + 1, string::npos);
  return dispatch_(tmp, newPattern, req, res);
}

bool HttpRouter::dispatch(HttpRequest *req, HttpResponse *res) {
  string pattern = req->request_line.url;
  if(pattern[0] ==  '/') pattern.erase(0, 1);
  LOGD("prepare to match url: %s", pattern.c_str());
  if(pattern.length() ==  0 && task != NULL) {
    LOGD("匹配到根路由");
    task(req, res);
    return true;
  }
  if(pattern[pattern.length() - 1] == '/') pattern.erase(pattern.length() - 1);
  return dispatch_(this, pattern, req, res);
}

bool HttpRouter::remove() {
  return false;
}
