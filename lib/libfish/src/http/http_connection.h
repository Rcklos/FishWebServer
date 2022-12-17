#pragma once

#include <memory.h>
#include "../socket_stream.h"
#include "http.h"

FISH_NAMESPACE_START

namespace http {

class HttpConnection : public SocketStream{
public:
    typedef std::shared_ptr<HttpConnection> Ptr;

public:
    HttpConnection(Socket::Ptr socket, bool auto_close = false);

    static HttpConnection::Ptr Create(Socket::Ptr socket, bool auto_close = false);

public:
    HttpResponse::Ptr RecvResponse();

    int SendRequest(HttpRequest::Ptr req);

};

} //namespace http
FISH_NAMESPACE_END
