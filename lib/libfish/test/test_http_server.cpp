#include "http/http_server.h"
using namespace fish;
using namespace fish::http;

int process_cnt = 0;
int client_handle_co_cnt = 0;
fish::Socket::Ptr g_listen_sock = nullptr;

void OnMainInt(int) {
    FISH_LOGDEBUG("OnMainInt");
    if (g_listen_sock) {
        g_listen_sock->Close();
    }
    kill(0, SIGINT);
    wait(nullptr);
    exit(-1);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: http_server process_count client_handle_co_cnt" << std::endl;
        exit(-1);
    }

    process_cnt = atoi(argv[1]);
    client_handle_co_cnt= atoi(argv[2]);

    fish::SetLogLevel(5);

    g_listen_sock = Socket::CreateTCP();
    assert(g_listen_sock);
    assert(g_listen_sock->Init());
    assert(g_listen_sock->Bind(IPv4Address::create("0.0.0.0", 9000)));
    assert(g_listen_sock->Listen(128));

    MultiProcess(process_cnt, [](){
            IoManager iom;
            auto http_server = HttpServer::Create("none", true);
            if (!http_server) {
                FISH_LOGFATAL("HttpServer::Create fail");
                exit(-1);
            }
            if (!http_server->Init(g_listen_sock, &iom, client_handle_co_cnt)) {
                FISH_LOGFATAL("HttpServer::Init fail");
                exit(-1);
            }
            if (!http_server->Start()) {
                FISH_LOGFATAL("HttpServer::Start fail");
                exit(-1);
            }

            auto servlet_dispatch = http_server->GetServletDispatch();
            servlet_dispatch->SetServlet("/", [](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
                rsp->SetBody("Servlet path is \"/\"\n");
                session->Close();
                return 0;
            });
            servlet_dispatch->SetServlet("/abc", [](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
                rsp->SetBody("Servlet path is \"/abc\"\n");
                return 0;
            });
            servlet_dispatch->SetGlobServlet("*", [](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
                rsp->SetBody("GlobServlet path is \"*\"\n");
                return 0;
            });

        iom.Start();
        }, OnMainInt);
    return 0;
}
