#include "kit/http/http_response.h"
#include <cstring>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include "log.h"
#include <unistd.h>
#include <cerrno>

using namespace fish;
using namespace std;

static map<int, string> const_status_text{
  {200, "OK"},
  {400, "Bad Request"},
  {404, "Not Found"},
  {500, "Internal Server Error"},
};


HttpResponse::HttpResponse(socket_t sockfd): sockfd(sockfd) {
  header["Content-Type"] = "text/plain; charset=UTF-8";
}
HttpResponse::~HttpResponse() {}

void HttpResponse::set_header(string key, string val) {
  header[key] = val;
}

void HttpResponse::set_header(const char* key, const char* val) {
  set_header(string(key), string(val));
}

void HttpResponse::set_header(char *key, char *val) {
  set_header(string(key), string(val));
}

bool HttpResponse::send_response_line() {
  string status_text = const_status_text[(int)response_line.status];
  stringstream ss;

  if(response_line.version ==  RESP_HTTP_1_1) ss << "HTTP/1.1" << " ";
  else if(response_line.version ==  RESP_HTTP_1_0) ss << "HTTP_1_0" << " ";
  else {
    LOGD("undefined reponse line version");
    return false;
  }

  ss << (int)response_line.status << " " << status_text << "\r\n";
  send_(ss);
  return true;
}

bool HttpResponse::send_header() {
  stringstream ss;
  for(map<string, string>::iterator it = header.begin();
      it != header.end(); it++) {
    ss << (*it).first << ": " << (*it).second << "\r\n";
  }
  ss << "\r\n";
  send_(ss);
  return true;
}

int HttpResponse::write(char *text) {
  if(!text) {
    if(!send_response_line()) return -1;
    if(!send_header()) return -2;
    return 1;
  }

  string text_(text);
  set_header("Content-Length", to_string(text_.length()));
  if(!send_response_line()) return -1;
  if(!send_header()) return -2;
  if(text) send_(text_);
  return 0;
}

int HttpResponse::write(const char * text) { return write(const_cast<char *>(text)); }

int HttpResponse::write(http_response_status_t status) {
  response_line.status = status;
  return write((char *)NULL);
}

bool HttpResponse::send_(const char *buff, size_t size) {
  if(sockfd == -1) return false;
  int ret = send(sockfd, buff, size, 0);
  if(ret == -1) {
    LOGD("response failed: %s", strerror(errno));
    end();
    return false;
  }
  return true;
}

inline bool HttpResponse::send_(stringstream &ss) {
  string str = ss.str();
  return send_(str);
}

inline bool HttpResponse::send_(string &s) {
  return send_(s.c_str(), s.length());
}

void HttpResponse::end() {
  if(sockfd > 0) close(sockfd);
  sockfd = -1;
}
