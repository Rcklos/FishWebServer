#include <string>
#include "tcp_server.h"
#include <assert.h>
#include <sys/wait.h>

using namespace std;
using namespace fish;

const std::string rsp = "HTTP/1.1 200 OK\r\nContent-length:9\r\n\r\nabcdefgh\n\r\n";
int process_cnt = 0;
int client_handle_co_cnt = 0;
Socket::Ptr g_listen_sock = nullptr;

class SimpleHttpServer : public TcpServer {
public:
    typedef std::shared_ptr<SimpleHttpServer> Ptr;
public:
    static SimpleHttpServer::Ptr Create() {
        return std::make_shared<SimpleHttpServer>();
    }

public:
    void ClientHandle(Socket::Ptr client) override {
        if (!client) {
            return ;
        }
        std::string req;
        req.resize(4096);
        while(client->Recv(&req[0], req.size()) > 0) {
            client->Send(rsp);
        }
    }
};

void OnMainInt(int) {
    FISH_LOGWARN("main process recv SIGINT");
    kill(0, SIGINT);
    wait(nullptr);
    exit(-1);
}

void OnChildInt(int) {
    FISH_LOGWARN("child process recv SIGINT");
    g_listen_sock->Close();
    exit(-1);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        FISH_LOGFATAL("Usage: test_tcp_server process_count max_client_cnt");
        exit(-1);
    }
    process_cnt = atoi(argv[1]);
    client_handle_co_cnt= atoi(argv[2]);
    signal(SIGINT, OnMainInt);

    //fish::SetLogLevel(5);
    g_listen_sock = Socket::CreateTCP();
    assert(g_listen_sock);
    assert(g_listen_sock->Init());
    assert(g_listen_sock->Bind(IPv4Address::create("0.0.0.0", 9000)));
    assert(g_listen_sock->Listen(128));

    for (int i = 0; i < process_cnt; ++i) {
        int ret = fork();
        if (ret != 0) {
            continue;
        } else {
            signal(SIGINT, OnChildInt);
            IoManager iom;
            auto shs = SimpleHttpServer::Create();
            if (!shs->Init(g_listen_sock, &iom, client_handle_co_cnt)) {
                FISH_LOGFATAL("SimpleHttpServer::Init fail");
                exit(-1);
            }
            if (!shs->Start()) {
                FISH_LOGFATAL("SimpleHttpServer::Start fail");
                exit(-1);
            }
            iom.Start();
        }
    }


    wait(nullptr);
}
