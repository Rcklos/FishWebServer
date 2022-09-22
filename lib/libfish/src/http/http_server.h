#pragma once

#include "tcp_server.h"
#include "http/servlet.h"

FISH_NAMESPACE_START
namespace http {

class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> Ptr;

protected:
    HttpServer(const std::string& name = "none", bool keepalive = false);

public:
    template<typename ...ArgsType>
    static HttpServer::Ptr Create(ArgsType ...args) {
        return std::shared_ptr<HttpServer>(new HttpServer(args...));
    }

public:
    void ClientHandle(Socket::Ptr client) override;

private:
    bool is_keep_alive_;
    ServletDispatch::Ptr servlet_dispatch_;

public:
    FUNCTION_BUILDER_VAR(ServletDispatch, servlet_dispatch_);
};

}//namespace http

FISH_NAMESPACE_END
