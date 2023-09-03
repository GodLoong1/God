#ifndef GOD_NET_SOCKET_H
#define GOD_NET_SOCKET_H

#include <sys/types.h>

#include "god/utils/NonCopyable.h"
#include "god/net/InetAddress.h"

namespace god
{

/// Tcp套接字
class Socket : NonCopyable
{
public:
    explicit Socket(int sockfd) noexcept
    : sockfd_(sockfd) { }

    ~Socket() noexcept
    {
        Close(sockfd_);
    }

    int fd() const noexcept
    {
        return sockfd_;
    }

    // 绑定地址
    void bind(const InetAddress& addr) noexcept;
    // 设置为监听
    void listen() noexcept;
    // 获取非阻塞连接(没有返回-1)
    int accept(InetAddress& peerAddr) noexcept;
    // 读
    ssize_t read(void* buf, size_t len) noexcept;
    // 写
    ssize_t write(const void* buf, size_t len) noexcept;
    // 关闭写端
    void shutdown() noexcept;
    // 地址复用
    void setReuseAddr(bool on) noexcept;
    // 端口复用
    void setReusePort(bool on) noexcept;
    // 关闭算法
    void setTcpNoDelay(bool on) noexcept;
    // 开启心跳
    void setKeepAlive(bool on) noexcept;

    // 获取非阻塞套接字
    static int Create(int family) noexcept;
    // 发起连接(返回0成功，返回-1重连)
    static int Connect(int sockfd, const InetAddress& serverAddr) noexcept;
    // 关闭套接字
    static void Close(int sockfd) noexcept;
    // 获取本端地址
    static InetAddress GetLocalAddr(int sockfd) noexcept;
    // 获取对端地址
    static InetAddress GetPeerAddr(int sockfd) noexcept;
    // 获取错误号
    static int GetSocketError(int sockfd) noexcept;
    // 是否为自连接
    static bool isSelfConnect(int sockfd) noexcept;

private:
    const int sockfd_;
};

} // namespace god

#endif