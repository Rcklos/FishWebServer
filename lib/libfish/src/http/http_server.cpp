#include "http_server.h"
#include "http_session.h"

const std::string rsp = "HTTP/1.1 200 OK\r\nContent-length:8\r\n\r\nabcdefgh\r\n";

FISH_NAMESPACE_START
namespace http {

HttpServer::HttpServer(const std::string &name, bool keepalive)
            : TcpServer(name),
              is_keep_alive_(keepalive),
              servlet_dispatch_(std::make_shared<ServletDispatch>()){
}

void HttpServer::ClientHandle(Socket::Ptr client) {
    if (!client) {
        return;
    }
    auto session = HttpSession::Create(client);
    while(session->IsConnected()) {
        auto req = session->RecvRequest();
        if (!req) {
            FISH_LOGDEBUG("Recv req fail");
            break;
        }
        bool close = !is_keep_alive_/* || req->GetIsClose()*/;
        auto rsp = HttpResponse::Create(req->GetVersion(), close);
        servlet_dispatch_->Handle(req, rsp, session);
        session->SendResponse(rsp);
        if (close) {
            break;
        }

    }
}

}//namespace http

FISH_NAMESPACE_END
