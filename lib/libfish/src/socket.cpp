#include "socket.h"
#include "iomanager.h"
#include "fdmanager.h"
#include "hook.h"
#include <netinet/tcp.h>

FISH_NAMESPACE_START

Socket::Socket(int family, int type, int protocol)
    : sockfd_(-1)
    , family_(family)
    , type_(type)
    , protocol_(protocol){
}

Socket::~Socket() {
    Close();
}

void Socket::Dump(std::ostream &os) const {
    os << EX_STRING_VARS(sockfd_, is_connected_, family_, type_, protocol_);
    if (local_address) {
        os << ", " << EX_STRING_VARS(local_address->to_string());
    }else {
        os << ", local_address=null";
    }
    if (remote_address) {
        os << ", " << EX_STRING_VARS(remote_address->to_string());
    }else {
        os << ", remote_address=null";
    }
}

bool Socket::Init(int sockfd) {
    if (sockfd != -1) {
        auto fdctx = FdManagerSgt::Instance().Get(sockfd);
        if (!fdctx || !fdctx->IsSocket() || fdctx->IsClosed()) {
            return false;
        }
        sockfd_ = sockfd;
    }
    InitSystemSocket(true);
    remote_address = GetRemoteAddress();
    local_address = GetLocalAddress();
    return true;
}

bool Socket::InitSystemSocket(bool auto_create) {
    if (!IsValid()) {
        if (!auto_create) {
            return false;
        }
        sockfd_ = socket(family_, type_, protocol_);
        if (sockfd_ == -1) {
            return false;
        }
    }
    SetOption(SOL_SOCKET, SO_REUSEADDR, 1);
    if (type_ == kTTcp) {
        SetOption(IPPROTO_TCP,  TCP_NODELAY, 1);// 禁用Nagle算法
    }
    return true;
}

Socket::Ptr Socket::CreateTCP(Address::Ptr address) {
    if (!address) {
        return std::make_shared<Socket>(kFIPv4, kTTcp, 0);
    }
    return std::make_shared<Socket>(address->get_family(), kTTcp, 0);
}

Socket::Ptr Socket::CreateUDP(Address::Ptr address) {
    if (!address) {
        return std::make_shared<Socket>(kFIPv4, kTUdp, 0);
    }
    return std::make_shared<Socket>(address->get_family(), kTUdp, 0);
}

Socket::Ptr Socket::CreateTCP6(Address::Ptr address) {
    if (!address) {
        return std::make_shared<Socket>(kFIPv6, kTTcp, 0);
    }
    return std::make_shared<Socket>(address->get_family(), kTTcp, 0);
}

Socket::Ptr Socket::CreateUDP6(Address::Ptr address) {
    if (!address) {
        return std::make_shared<Socket>(kFIPv6, kTUdp, 0);
    }
    return std::make_shared<Socket>(address->get_family(), kTUdp, 0);
}

Socket::Ptr Socket::CreateUnixTCP() {
    return std::make_shared<Socket>(kFUnix, kTTcp, 0);
}

Socket::Ptr Socket::CreateUnixUDP() {
    return std::make_shared<Socket>(kFUnix, kTUdp, 0);
}

Socket::Ptr Socket::Accept() {
    auto remote_sock = std::make_shared<Socket>(family_, type_, protocol_);
    int remote_fd = accept(sockfd_, nullptr, nullptr);
    if (remote_fd == -1) {
        return nullptr;
    }
    if (!remote_sock->Init(remote_fd)) {
        return nullptr;
    }
    remote_sock->is_connected_ = true;
    return remote_sock;
}

bool Socket::Bind(const Address::Ptr addr) {
    if (!addr) {
        return false;
    }
    if (!IsValid()) {
        InitSystemSocket(true);
        if (FISH_UNLICKLY(!IsValid())) {
            return false;
        }
    }

    if (FISH_UNLICKLY(addr->get_family() != family_)) {
        return false;
    }

    if (bind(sockfd_, addr->get_addr(), addr->size())) {
        return false;
    }

    return true;
}

bool Socket::Connect(const Address::Ptr dest, int64_t time_out_ms) {
    if (!dest) {
        return false;
    }

    if (!IsValid()) {
        InitSystemSocket(true);
        if (FISH_UNLICKLY(!IsValid())) {
            return false;
        }
    }

    if (FISH_UNLICKLY(dest->get_family() != family_)) {
        return false;
    }

    if (time_out_ms == -1) {
        if (connect(sockfd_, dest->get_addr(), dest->size())) {
            return false;
        }
    }else {
        if (connect_with_timeout(sockfd_, dest->get_addr(), dest->size(), time_out_ms)) {
            return false;
        }
    }
    is_connected_ = true;
    remote_address = GetRemoteAddress();
    local_address = GetLocalAddress();
    return true;
}

bool Socket::Listen(int backlog) {
    if (!IsValid()) {
        InitSystemSocket(true);
        if (FISH_UNLICKLY(!IsValid())) {
            return false;
        }
    }

    if (listen(sockfd_, backlog)) {
        return false;
    }

    return true;
}

bool Socket::Close() {
    if (!is_connected_) {
        return true;
    }
    if (IsValid()) {
        if (close(sockfd_)) {
            return false;
        }
        sockfd_ = -1;
    }
    is_connected_ = false;
    return true;
}

int Socket::Send(const char *buffer, size_t len, int flags) {
    if (!IsConnected() || !buffer) {
        return -1;
    }
    return send(sockfd_, buffer, len, flags);
}

int Socket::Send(const std::string &buffer, int flags) {
    if (!IsConnected()) {
        return -1;
    }
    return Send(buffer.c_str(), buffer.size(), flags);
}

int Socket::Send(const iovec *iov, size_t iov_len, int flags) {
    if (!IsConnected() || !iov) {
        return -1;
    }
    msghdr msg;
    MemSetZero(msg);
    msg.msg_iov = (iovec*)iov;
    msg.msg_iovlen = iov_len;
    return sendmsg(sockfd_, &msg, flags);
}

int Socket::SendTo(const Address::Ptr to, const char *buffer, size_t len, int flags) {
    if (!IsConnected() || !to || !buffer) {
        return -1;
    }
    return sendto(sockfd_, buffer, len, flags, to->get_addr(), to->size());
}

int Socket::SendTo(const Address::Ptr to, const std::string &buffer, int flags) {
    return SendTo(to, buffer.c_str(), buffer.size(), flags);
}

int Socket::SendTo(const Address::Ptr to, const iovec *iov, size_t iov_len, int flags) {
    if (!IsConnected() || !to || !iov) {
        return -1;
    }
    msghdr msg;
    MemSetZero(msg);
    msg.msg_iov = (iovec*)iov;
    msg.msg_iovlen = iov_len;
    msg.msg_name = (void*)to->get_addr();
    msg.msg_namelen = to->size();
    return sendmsg(sockfd_, &msg, flags);
}

int Socket::Recv(char *buffer, size_t len, int flags) {
    if (!IsConnected() || !buffer) {
        return -1;
    }
    return recv(sockfd_, buffer, len, flags);
}

int Socket::Recv(std::string &buffer, int flags) {
    if (!IsConnected()) {
        return -1;
    }
    char buf[4096];
    MemSetZero(buf);
    int ret = Recv(buf, sizeof(buf), flags);
    buffer = buf;
    return ret;
}

int Socket::Recv(iovec *iov, size_t iov_len, int flags) {
    if (!IsConnected() || !iov) {
        return -1;
    }
    msghdr msg;
    MemSetZero(msg);
    msg.msg_iov = iov;
    msg.msg_iovlen = iov_len;
    return recvmsg(sockfd_, &msg, flags);
}

int Socket::RecvFrom(Address::Ptr from, const char *buffer, size_t len, int flags) {
    if (!IsConnected() || !from || !buffer) {
        return -1;
    }
    socklen_t socklen = from->size();
    return recvfrom(sockfd_, (void *)buffer, len, flags, (sockaddr*)from->get_addr(), &socklen);
}

int Socket::RecvFrom(Address::Ptr from, std::string &buffer, int flags) {
    if (!IsConnected() || !from) {
        return -1;
    }
    char buf[4096];
    MemSetZero(buf);
    socklen_t socklen = from->size();
    int ret = recvfrom(sockfd_, buf, sizeof(buf), flags, (sockaddr*)from->get_addr(), &socklen);
    buffer = buf;
    return ret;
}

int Socket::RecvFrom(Address::Ptr from, iovec *iov, size_t iov_len, int flags) {
    if (!IsConnected() || !from || !iov) {
        return -1;
    }
    msghdr msg;
    MemSetZero(msg);
    msg.msg_iov = iov;
    msg.msg_iovlen = iov_len;
    msg.msg_name = (void*)from->get_addr();
    msg.msg_namelen = from->size();
    return recvmsg(sockfd_, &msg, flags);
}

int Socket::GetFamily() const {
    return family_;
}

int Socket::GetType() const {
    return type_;
}

int Socket::GetProtocol() const {
    return protocol_;
}

bool Socket::IsConnected() {
    return is_connected_;
}

bool Socket::IsValid() {
    return IsValidSockfd(sockfd_);
}

int Socket::GetError() const {
    int error = 0;
    socklen_t len = sizeof(error);
    if (!GetOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        error = errno;
    }
    return error;
}

int64_t Socket::GetSendTimeOut() const {
    auto ptr = FdManagerSgt::Instance().Get(sockfd_);
    if (ptr) {
        return ptr->GetTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::SetSendTimeOut(int64_t time_out) {
    timeval tv{time_out / 1000, time_out % 1000};
    SetOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int64_t Socket::GetRecvTimeOut() const {
    auto ptr = FdManagerSgt::Instance().Get(sockfd_);
    if (ptr) {
        return ptr->GetTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::SetRecvTimeOut(int64_t time_out) {
    timeval tv{time_out / 1000, time_out % 1000};
    SetOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

std::ostream& operator<<(std::ostream& os, const Socket& socket) {
    socket.Dump(os);
    return os;
}

int Socket::GetSockfd() const {
    return sockfd_;
}

Address::Ptr Socket::GetLocalAddress() {
    if (local_address) {
        return local_address;
    }
    Address::Ptr addr;
    switch (family_) {
        case AF_INET: {
            addr = std::make_shared<IPv4Address>();
            break;
        }
        case AF_INET6: {
            // addr = std::make_shared<IPv6Address>();
            break;
        }
        case AF_UNIX: {
            addr = std::make_shared<UnixAddress>();
            break;
        }
        default: {
            // addr = std::make_shared<UnknownAddress>();
            break;
        }
    }
    auto addrlen = addr->size();
    if (getsockname(sockfd_, (sockaddr*)addr->get_addr(), &addrlen)) {
        // return std::make_shared<UnknownAddress>();
        return nullptr;
    }
    if (family_ == AF_UNIX) {
        auto unix_addr = std::dynamic_pointer_cast<UnixAddress>(addr);
        if (unix_addr) {
            unix_addr->set_size(addrlen);
        }
    }
    return addr;
}

Address::Ptr Socket::GetRemoteAddress() {
    if (remote_address) {
        return remote_address;
    }
    Address::Ptr addr;
    switch (family_) {
        case AF_INET: {
            addr = std::make_shared<IPv4Address>();
            break;
        }
        case AF_INET6: {
            // TODO: 实现ipv6
            // addr = std::make_shared<IPv6Address>();
            break;
        }
        case AF_UNIX: {
            addr = std::make_shared<UnixAddress>();
            break;
        }
        default: {
            // addr = std::make_shared<UnknownAddress>();
            break;
        }
    }
    auto addrlen = addr->size();
    if (getpeername(sockfd_, (sockaddr*)addr->get_addr(), &addrlen)) {
        // return std::make_shared<UnknownAddress>();
        return nullptr;
    }
    if (family_ == AF_UNIX) {
        auto unix_addr = std::dynamic_pointer_cast<UnixAddress>(addr);
        if (unix_addr) {
            unix_addr->set_size(addrlen);
        }
    }
    return addr;
}

bool Socket::CancelRead() {
    auto iom = IoManager::GetCurIoManager();
    if (iom) {
        return iom->TrgEvent(sockfd_, EPOLLIN);
    }
    return false;
}

bool Socket::CancelWrite() {
    auto iom = IoManager::GetCurIoManager();
    if (iom) {
        return iom->TrgEvent(sockfd_, EPOLLOUT);
    }
    return false;
}

bool Socket::CancelAccept() {
    auto iom = IoManager::GetCurIoManager();
    if (iom) {
        return iom->TrgEvent(sockfd_, EPOLLIN);
    }
    return false;
}

bool Socket::CancelAll() {
    auto iom = IoManager::GetCurIoManager();
    if (iom) {
        return iom->CncAllEvent(sockfd_);
    }
    return true;
}

bool Socket::GetOption(int level, int option, void *val, socklen_t *len) const {
    int ret = getsockopt(sockfd_, level, option, val, len);
    if (ret) {
        return false;
    }
    return true;
}

bool Socket::SetOption(int level, int option, const void *val, socklen_t len) {
    int ret = setsockopt(sockfd_, level, option, val, len);
    if (ret) {
        return false;
    }
    return true;
}

FISH_NAMESPACE_END
