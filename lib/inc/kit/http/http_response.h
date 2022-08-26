#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__
#include "kit/sock.h"
#include <cstddef>
#include <map>
#include <string>

namespace fish {

typedef enum{
  RESP_HTTP_1_0,
  RESP_HTTP_1_1
} http_response_version_t;

typedef enum {
  SC_OK = 200,
  SC_BAD_REQUEST = 400,
  SC_NOT_FOUND = 404,
  SC_INTE_SERVER_ERROR = 500
} http_response_status_t;

typedef struct http_response_line_t {
  http_response_version_t version;
  int status;

  http_response_line_t():
    version(RESP_HTTP_1_1),
    status(SC_OK) {}
} http_response_line_t;

static int response_line_to_text(http_response_version_t &response_line, char *buff, int offset);

class HttpResponse {
private:
  socket_t sockfd;
  http_response_line_t response_line;
  std::map<std::string, std::string> header;

public:
  HttpResponse(socket_t sockfd);
  ~HttpResponse();
  void set_header(std::string key, std::string value);
  void set_header(const char *key, const char *value);
  void set_header(char *key, char *value);
  int write(http_response_status_t status);
  int write(const char *text);
  int write(char *text);
  int write(std::string text);
  int write();
  void end();
private:
  bool send_response_line();
  bool send_header();
  bool send_(const char *buff, size_t size);
  inline bool send_(std::stringstream &ss);
  inline bool send_(std::string &s);
};

}
#endif /* __HTTP_RESPONSE_H__ */
