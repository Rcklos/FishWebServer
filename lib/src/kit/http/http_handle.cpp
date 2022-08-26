#include "kit/http/http_handle.h"
#include "kit/http/http_request.h"
#include "kit/http/http_response.h"
#include "kit/http/http_router.h"
#include "kit/thread_pool.h"
#include "log.h"
#include <cassert>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/socket.h>

using namespace fish;
using namespace std;

HttpHandle::HttpHandle() {
  tpool_ = new ThreadPool(5);
  router = HttpRouter::createRoot(NULL);
}

HttpHandle::~HttpHandle() {
  if(tpool_ != NULL) delete tpool_;
  tpool_ = NULL;
}

void parse_http(char *buff) {
  string str(buff);
  // LOGD("parse http: %s", str.c_str());
  stringstream ss(str);
  string ans;
  struct {
    string method;
    string path;
    string version;
  } first_line;
  if(std::getline(ss, ans)) {
    auto pos = ans.find_first_of(" ");
    assert(pos != std::string::npos);
    first_line.method = ans.substr(0, pos);
    LOGD("method: %s", first_line.method.c_str());
    ans = ans.substr(pos + 1, std::string::npos);
    pos = ans.find_first_of(" ");
    assert(pos != std::string::npos);
    first_line.path = ans.substr(0, pos);
    LOGD("path: %s", first_line.path.c_str());
    ans = ans.substr(pos + 1, std::string::npos);
    pos = ans.find_first_of(" ");
    first_line.version = ans.substr(0, pos);
    LOGD("version: %s", first_line.version.c_str());
  }
}

void HttpHandle::handle_(socket_t sockfd, char *buff_, int size) {
  // parse_http(buff_);
  HttpRequest *request = HttpRequest::parse_request_info(buff_);
  if(!request) {
    LOGD("parse ----> bad request");
    string bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
    int ret = send(sockfd, bad_request.c_str(), bad_request.length(), 0);
    if(ret == -1)
      LOGD("send http response of bad request(400) failed");
    if(sockfd > 0) close(sockfd);
    return;
  }
  // string str = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\ntest\r\n\r\n";
  // int ret = send(sockfd, str.c_str(), str.length(), 0);
  // LOGD("send: ret=%d, errno: %s", ret, strerror(errno));

  HttpResponse response(sockfd);
  // int ret = response.write("你好啊\n");
  // if(ret < 0)
  //   LOGD("hanlde --> response failed: ret = %d", ret);

  if(!router->dispatch(request, &response)) {
    LOGD("hanlde --> url[%s] not match router!", request->request_line.url.c_str());
    if(response.write(SC_NOT_FOUND) < 0) {
      LOGD("response write failed!");
    }
    response.end();
  }

  delete buff_;
}

bool HttpHandle::route(string pattern, const RouteTask &task) {
  return router->route(pattern, task);
}

void HttpHandle::handle(socket_t sockfd, char *buff_, int size) {
  char *buff = new char[1024];
  memccpy(buff, buff_, 0, size);
  tpool_->addLast(std::bind(&HttpHandle::handle_, this, sockfd, buff, size));
}
