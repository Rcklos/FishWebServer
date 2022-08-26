#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <map>
#include <string>

namespace fish {

typedef enum {
  HTTP_METHOD_GET,
  HTTP_METHOD_POST
} http_request_method_t;

typedef enum{
  HTTP_1_0,
  HTTP_1_1
} http_request_version_t;

typedef struct {
  http_request_method_t method;
  std::string url;
  http_request_version_t version;
} http_request_line_t;

class HttpRequest {
  public:
    http_request_line_t request_line;
    std::map<std::string, std::string> header;

    static HttpRequest* parse_request_info(char *buff);
};

}
#endif /* __HTTP_REQUEST_H__ */
