#include "god/net/TcpBuffer.h"

#include <endian.h>
#include <sys/uio.h>

#include <cassert>
#include <cstring>

namespace god
{

TcpBuffer::TcpBuffer(size_t len) noexcept
: start_(new char[len]),
  read_(start_),
  write_(start_),
  end_(start_ + len)
{
    ::memset(start_, 0, len * sizeof(char));
}

TcpBuffer::~TcpBuffer() noexcept
{
    delete[] start_;
}

uint8_t TcpBuffer::peekInt8() const noexcept
{
    assert(readByte() >= sizeof(uint8_t));
    return *(const uint8_t*)readPeek();
}

uint16_t TcpBuffer::peekInt16() const noexcept
{
    assert(readByte() >= sizeof(uint16_t));
    uint16_t val = *(const uint16_t*)readPeek();
    return ::be16toh(val);
}

uint32_t TcpBuffer::peekInt32() const noexcept
{
    assert(readByte() >= sizeof(uint32_t));
    uint32_t val = *(const uint32_t*)readPeek();
    return ::be32toh(val);
}

uint64_t TcpBuffer::peekInt64() const noexcept
{
    assert(readByte() >= sizeof(uint64_t));
    uint64_t val = *(const uint64_t*)readPeek();
    return ::be64toh(val);
}

uint8_t TcpBuffer::readInt8() noexcept
{
    uint8_t ret = peekInt8();
    retrieve(sizeof(ret));
    return ret;
}

uint16_t TcpBuffer::readInt16() noexcept
{
    uint16_t ret = peekInt16();
    retrieve(sizeof(ret));
    return ret;
}

uint32_t TcpBuffer::readInt32() noexcept
{
    uint32_t ret = peekInt32();
    retrieve(sizeof(ret));
    return ret;
}

uint64_t TcpBuffer::readInt64() noexcept
{
    uint64_t ret = peekInt64();
    retrieve(sizeof(ret));
    return ret;
}

void TcpBuffer::writeInt8(const uint8_t val) noexcept
{
    write((const char*)&val, sizeof(uint8_t));
}

void TcpBuffer::writeInt16(const uint16_t val) noexcept
{
    uint16_t ret = ::htobe16(val);
    write((const char*)&ret, sizeof(uint16_t));
}

void TcpBuffer::writeInt32(const uint32_t val) noexcept
{
    uint32_t ret = ::htobe32(val);
    write((const char*)&ret, sizeof(uint32_t));
}

void TcpBuffer::writeInt64(const uint64_t val) noexcept
{
    uint64_t ret = ::htobe64(val);
    write((const char*)&ret, sizeof(uint64_t));
}

void TcpBuffer::write(const char* buf, size_t len) noexcept
{
    if (writeByte() < len)
    {
        reallocate(len);
    }
    memcpy(write_, buf, len);
    write_ += len;
}

void TcpBuffer::write(const std::string_view& buf) noexcept
{
    write(buf.data(), buf.size());
}

ssize_t TcpBuffer::readFd(int sockfd) noexcept
{
    char buf[8192];
    struct iovec iov[2];
    size_t writeSize = writeByte();

    iov[0].iov_base = write_;
    iov[0].iov_len = writeSize;
    iov[1].iov_base = buf;
    iov[1].iov_len = sizeof(buf);

    ssize_t n = ::readv(sockfd, iov, sizeof(iov) / sizeof(*iov));
    if (n > 0)
    {
        if (static_cast<size_t>(n) <= writeSize)
        {
            write_ += n;
        }
        else
        {
            write_ = end_;
            write(buf, n - writeSize);
        }
    }
    return n;
}

std::string TcpBuffer::read(size_t len) noexcept
{
    assert(len <= readByte());
    std::string ret(readPeek(), len);
    retrieve(len);
    return ret;
}

std::string TcpBuffer::readUntil(const char* end) noexcept
{
    assert(readPeek() <= end);
    assert(end <= writePeek());
    return read(end - readPeek());
}

std::string TcpBuffer::readAll() noexcept
{
    return read(readByte());
}

void TcpBuffer::retrieve(size_t len) noexcept
{
    assert(len <= readByte());
    if (len < readByte())
    {
        read_ += len;
    }
    else
    {
        retrieveAll();
    }
}

void TcpBuffer::retrieveUntil(const char* end) noexcept
{
    assert(readPeek() <= end);
    assert(end <= writePeek());

    retrieve(end - readPeek());
}

void TcpBuffer::retrieveAll() noexcept
{
    read_ = start_;
    write_ = start_;
}

void TcpBuffer::reallocate(size_t len) noexcept
{
    const size_t readSize = readByte();

    // 重新分配
    if (size() - readByte() < len)
    {
        const size_t newSize = (size() + len) * 2;
        char* newData = new char[newSize];

        ::memcpy(newData, read_, readSize);
        delete[] start_;

        start_ = newData;
        read_ = newData;
        write_ = read_ + readSize;
        end_ = start_ + newSize;
    }
    else // 将数据往头部移动
    {
        ::memmove(start_, read_, readSize);

        read_ = start_;
        write_ = read_ + readSize;
    }
}

} // namespace god