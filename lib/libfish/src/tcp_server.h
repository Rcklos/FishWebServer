#pragma once

#include <memory>
#include <vector>
#include <map>
#include "iomanager.h"
#include "socket.h"
#include "address.h"
#include "common.h"

FISH_NAMESPACE_START

class TcpServer : public std::enable_shared_from_this<TcpServer>,
                  Noncopyable,
                  public BaseDump{
public:
    typedef std::shared_ptr<TcpServer> Ptr;

    struct Task {
        typedef std::shared_ptr<Task> Ptr;
        Task(Coroutine::Ptr c, Socket::Ptr& cl) : co(c), cli(cl){}
        Coroutine::Ptr      co  = nullptr;
        Socket::Ptr&        cli;
    };

protected:
    /**
     * @brief 构造函数 (不开放构造，防止share_from_this()访问失败)
     * @param[in] worker: socket客户端工作的协程调度器
     * @param[in] io_worder: todo
     * @param[in] accept_worker: 服务器接受连接的协程调度器
     * @return
     */
    TcpServer(const std::string& name = "none");

public:
    static TcpServer::Ptr Create(const std::string& name = "none");

    /**
     * @brief 析构函数
     */
    virtual ~TcpServer();

public:
    void Dump(std::ostream &os) const override;

public:
    virtual bool Init(Address::Ptr address, IoManager* io_manager = IoManager::GetCurIoManager(), uint32_t client_handler_cnt = 512/*, bool ssl = false*/);
    virtual bool Init(Socket::Ptr socket, IoManager* io_manager = IoManager::GetCurIoManager(), uint32_t client_handler_cnt = 512/*, bool ssl = false*/);
    virtual bool Bind(const std::vector<Address::Ptr>& addrs/*, BOOL SSL = FALSE*/);
    virtual bool Start();
    virtual void Stop();

    virtual void ClientHandle(Socket::Ptr client);
    virtual void OnAccept(Socket::Ptr listen_socket);

public:
    size_t GetSocketNum(){ return sockets_.size(); }

private:
    void OnClientHandle(Socket::Ptr client);

private:
    std::string                 name_                   = "none";
    IoManager*                  io_manager_             = nullptr;
    int64_t                     accept_timeout_         = -1;
    uint32_t                    client_handler_cnt_     = 512;
    std::string                 type_                   = "tcp";
    bool                        is_stop_                = true;
    bool                        ssl_                    = false;
    bool                        is_init_                = false;
    std::queue<Task::Ptr>       idle_cos_;
    std::vector<Socket::Ptr>    sockets_;

public:
    FUNCTION_BUILDER_VAR(IsStop, is_stop_);
};
FISH_NAMESPACE_END
