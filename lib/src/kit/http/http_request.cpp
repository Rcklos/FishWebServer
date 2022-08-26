#include "kit/http/http_request.h"
#include <assert.h>
#include "log.h"
#include <sstream>
#include <strings.h>

using namespace fish;
using namespace std;

bool parse_request_line(stringstream &ss, http_request_line_t &request_line) {
  string method, url, version, ans;
  if(std::getline(ss, ans)) {
    if(ans[ans.length() - 1] == '\r') ans.erase(ans.length() - 1);
    auto pos = ans.find_first_of(" ");
    if(pos ==  std::string::npos) return false;
    method = ans.substr(0, pos);
    ans = ans.substr(pos + 1, std::string::npos);
    pos = ans.find_first_of(" ");
    if(pos ==  string::npos) return false;
    url = ans.substr(0, pos);
    ans = ans.substr(pos + 1, std::string::npos);
    pos = ans.find_first_of(" ");
    version = ans.substr(0, pos);
  }
  else return false;

  if(version == "HTTP/1.1") request_line.version = fish::HTTP_1_1;
  else if(version ==  "HTTP/1.0") request_line.version = fish::HTTP_1_0;
  else return false;
  LOGD("parse: --> version: %s", version.c_str());

  if(strncasecmp(method.c_str(), "GET", method.length()))
    request_line.method = fish::HTTP_METHOD_GET;
  else if (strncasecmp(method.c_str(), "POST", method.length())) 
    request_line.method = fish::HTTP_METHOD_POST;
  else return false;
  LOGD("parse: --> method: %s", method.c_str());

  request_line.url = url;

  LOGD("parse --> url: %s", url.c_str());
  return true;
}

bool parse_request_header(stringstream &ss, 
  std::map<string, string> &header) {

  string ans, key, value;
  string::size_type pos;
  while(std::getline(ss, ans) && ans != "\r") {
    if(ans[ans.length() - 1] ==  '\r') {
      ans.erase(ans.length() - 1);
      pos = ans.find_first_of(": ");
      if(pos ==  string::npos) return false;
      key = ans.substr(0, pos);
      value = ans.substr(pos + 2);
      header[key] = value;
    }
  }

  for(map<string, string>::iterator it = header.begin(); 
      it != header.end(); it++) {
    LOGD("parse header --> %s: %s", (*it).first.c_str(), (*it).second.c_str());
  }

  return true;
}

HttpRequest* HttpRequest::parse_request_info(char *buff) {
  string str(buff);
  std::stringstream ss(str);

  HttpRequest *request = new HttpRequest();
  if(!parse_request_line(ss, request->request_line)) {
    delete request;
    return NULL;
  }

  if(!parse_request_header(ss, request->header)) {
    delete request;
    return NULL;
  }

  return request;
}
