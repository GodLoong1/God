#ifndef GOD_NET_TCPBUFFER_H
#define GOD_NET_TCPBUFFER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "god/utils/NonCopyable.h"

namespace god
{

/// Tcp缓冲区
class TcpBuffer : NonCopyable
{
public:
    explicit TcpBuffer(size_t len = 2048) noexcept;
    ~TcpBuffer() noexcept;

    size_t headByte() const noexcept
    {
        return read_ - start_;
    }

    size_t readByte() const noexcept
    {
        return write_ - read_;
    }

    size_t writeByte() const noexcept
    {
        return end_ - write_;
    }

    const char* readPeek() const noexcept
    {
        return read_;
    }

    const char* writePeek() const noexcept
    {
        return write_;
    }

    bool empty() const noexcept
    {
        return read_ == write_;
    }

    const char* data() const noexcept
    {
        return readPeek();
    }

    size_t size() const noexcept
    {
        return readByte();
    }

public:
    uint8_t peekInt8() const noexcept;
    uint16_t peekInt16() const noexcept;
    uint32_t peekInt32() const noexcept;
    uint64_t peekInt64() const noexcept;

    uint8_t readInt8() noexcept;
    uint16_t readInt16() noexcept;
    uint32_t readInt32() noexcept;
    uint64_t readInt64() noexcept;

    void writeInt8(const uint8_t val) noexcept;
    void writeInt16(const uint16_t val) noexcept;
    void writeInt32(const uint32_t val) noexcept;
    void writeInt64(const uint64_t val) noexcept;

public:
    void write(const char* buf, size_t len) noexcept;
    void write(const std::string_view& buf) noexcept;
    ssize_t readFd(int sockfd) noexcept;

    std::string read(size_t len) noexcept;
    std::string readUntil(const char* end) noexcept;
    std::string readAll() noexcept;

    void retrieve(size_t len) noexcept;
    void retrieveUntil(const char* end) noexcept;
    void retrieveAll() noexcept;

private:
    void reallocate(size_t len) noexcept;

    char* start_;
    char* read_;
    char* write_;
    char* end_;
};

} // namespace god

#endif