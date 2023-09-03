#include "god/net/Socket.h"

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "god/utils/Logger.h"

namespace god
{

void Socket::bind(const InetAddress& addr) noexcept
{
    if (::bind(sockfd_, addr.getSockAddr(), addr.getSockLen()) < 0)
    {
        LOG_FATAL << strerr();
    }
}

void Socket::listen() noexcept
{
    if (::listen(sockfd_, SOMAXCONN) < 0)
    {
        LOG_FATAL << strerr();
    }
}

int Socket::accept(InetAddress& peerAddr) noexcept
{
    socklen_t len = peerAddr.getSockLen();
    int connfd = ::accept4(sockfd_, peerAddr.getSockAddr(), &len,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0 && errno != EAGAIN && errno != EINTR)
    {
        LOG_FATAL << strerr();
    }
    return connfd;
}

ssize_t Socket::read(void* buf, size_t len) noexcept
{
    return ::read(sockfd_, buf, len);
}

ssize_t Socket::write(const void* buf, size_t len) noexcept
{
    return ::write(sockfd_, buf, len);
}

void Socket::shutdown() noexcept
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR << strerr();
    }
}

void Socket::setReuseAddr(bool on) noexcept
{
    int opt = on;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                     &opt, sizeof(opt)) < 0)
    {
        LOG_FATAL << strerr();
    }
}

void Socket::setReusePort(bool on) noexcept
{
    int opt = on;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                     &opt, sizeof(opt)) < 0)
    {
        LOG_FATAL << strerr();
    }
}

void Socket::setTcpNoDelay(bool on) noexcept
{
    int opt = on;
    if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                     &opt, sizeof(opt)) < 0)
    {
        LOG_FATAL << strerr();
    }
}

void Socket::setKeepAlive(bool on) noexcept
{
    int opt = on;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                     &opt, sizeof(opt)) < 0)
    {
        LOG_FATAL << strerr();
    }
}

int Socket::Create(int family) noexcept
{
    int sockfd = ::socket(family,
                          SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL << strerr();
    }
    return sockfd;
}

int Socket::Connect(int sockfd, const InetAddress& serverAddr) noexcept
{
    int ret = ::connect(sockfd, serverAddr.getSockAddr(),
                        serverAddr.getSockLen());
    int err = ret ? errno : 0;
    switch (err)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
        {
            ret = 0;
            break;
        }
        case EAGAIN:
        case ECONNREFUSED:
        case ENETUNREACH:
        {
            ret = -1;
            break;
        }
        default:
        {
            LOG_FATAL << strerr();
            break;
        }
    }
    return ret;
}

void Socket::Close(int sockfd) noexcept
{
    if (::close(sockfd) < 0)
    {
        LOG_FATAL << strerr();
    }
}

InetAddress Socket::GetLocalAddr(int sockfd) noexcept
{
    InetAddress addr;
    socklen_t len = addr.getSockLen();

    if (::getsockname(sockfd, addr.getSockAddr(), &len) < 0)
    {
        LOG_ERROR << strerr();
    }
    return addr;
}

InetAddress Socket::GetPeerAddr(int sockfd) noexcept
{
    InetAddress addr;
    socklen_t len = addr.getSockLen();

    if (::getpeername(sockfd, addr.getSockAddr(), &len) < 0)
    {
        LOG_ERROR << strerr();
    }
    return addr;
}

int Socket::GetSocketError(int sockfd) noexcept
{
    int opt;
    socklen_t len = sizeof(opt);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &opt, &len) < 0)
    {
        return errno;
    }
    return opt;
}

bool Socket::isSelfConnect(int sockfd) noexcept
{
    return GetLocalAddr(sockfd) == GetPeerAddr(sockfd);
}

} // namespace god
