#pragma once

#include "stream.h"
#include "socket.h"
#include "common.h"

FISH_NAMESPACE_START

class SocketStream : public Stream, public BaseDump{
public:
    typedef std::shared_ptr<SocketStream> Ptr;

public:
    SocketStream(Socket::Ptr socket, bool auto_close = false);
    ~SocketStream();

public:
    int Read(void *buffer, size_t length) override;

    int Read(ByteArray::Ptr ba, size_t length) override;

    int Write(const void *buffer, size_t length) override;

    int Write(ByteArray::Ptr ba, size_t length) override;

    void Close() override;

    bool IsConnected() const;

private:
    Socket::Ptr socket_;
    bool auto_close_;

public:
    FUNCTION_BUILDER_VAR_GETTER(Socket, socket_);
};

FISH_NAMESPACE_END

