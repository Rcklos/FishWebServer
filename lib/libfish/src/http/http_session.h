#pragma once

#include <memory>
#include "../socket_stream.h"
#include "http.h"

FISH_NAMESPACE_START
namespace http {

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> Ptr;

public:
    HttpSession(Socket::Ptr socket, bool auto_close = false);

    static HttpSession::Ptr Create(Socket::Ptr socket, bool auto_close = false);

public:
    HttpRequest::Ptr RecvRequest();

    int SendResponse(HttpResponse::Ptr rsp);

};

}//namespace http

FISH_NAMESPACE_END
