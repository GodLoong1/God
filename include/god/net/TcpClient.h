#ifndef GOD_NET_TCPCLIENT_H
#define GOD_NET_TCPCLIENT_H

#include <atomic>
#include <mutex>

#include "god/utils/NonCopyable.h"
#include "god/net/EventLoop.h"
#include "god/net/InetAddress.h"
#include "god/net/Connector.h"
#include "god/net/TcpConnection.h"

namespace god
{

/// Tcp客户端
class TcpClient : NonCopyable
{
public:
    TcpClient(EventLoop* loop,
              const InetAddress& serverAddr,
              const std::string_view& name = "TcpClient") noexcept;
    ~TcpClient();

    void start() noexcept;
    void stop() noexcept;

    const std::string name() const noexcept
    {
        return name_;
    }

    const TcpConnectionPtr& connection() const noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    void setConnectionCallback(ConnectionCallback&& cb) noexcept
    {
        connectionCallback_ = std::move(cb);
    }

    void setMessageCallback(MessageCallback&& cb) noexcept
    {
        messageCallback_ = std::move(cb);
    }

    void setRetry(bool on) noexcept
    {
        retry_ = on;
    }

private:
    void newConnection(int sockfd) noexcept;
    void removeConnection(const TcpConnectionPtr& conn) noexcept;

    EventLoop* const loop_;
    const std::string name_;
    std::unique_ptr<Connector> connector_;
    bool started_{false};
    bool retry_{false};

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;

    mutable std::mutex mutex_;
    TcpConnectionPtr connection_;
};

} // namespace god

#endif