#ifndef GOD_NET_TCPCONNECTION_H
#define GOD_NET_TCPCONNECTION_H

#include <functional>
#include <memory>
#include <string>

#include "god/utils/NonCopyable.h"
#include "god/utils/Date.h"
#include "god/net/InetAddress.h"
#include "god/net/Socket.h"
#include "god/net/TcpBuffer.h"

namespace god
{

class EventLoop;
class Channel;
class TimerWheel;

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

/// 连接回调
using ConnectionCallback =
    std::function<void(const TcpConnectionPtr& conn)>;

/// 消息回调
using MessageCallback =
    std::function<void(const TcpConnectionPtr& conn, TcpBuffer& buf)>;

/// 关闭回调
using CloseCallback =
    std::function<void(const TcpConnectionPtr& conn)>;

/// Tcp连接
class TcpConnection : NonCopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr) noexcept;
    ~TcpConnection() noexcept;

    void init() noexcept;

    EventLoop* getLoop() const noexcept
    {
        return loop_;
    }

    int fd() const noexcept
    {
        return socket_.fd();
    }

    const InetAddress& localAddr() const noexcept
    {
        return localAddr_;
    }

    const InetAddress& peerAddr() const noexcept
    {
        return peerAddr_;
    }

    bool isConnected() const noexcept
    {
        return status_ == kConnected;
    }

    bool isDisconnected() const noexcept
    {
        return status_ ==  kDisconnected;
    }

    void setConnectionCallback(const ConnectionCallback& cb) noexcept
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb) noexcept
    {
        messageCallback_ = cb;
    }

    void setCloseCallback(const CloseCallback& cb) noexcept
    {
        closeCallback_ = cb;
    }

    void setContext(std::shared_ptr<void>&& any) noexcept
    {
        context_= std::move(any);
    }

    template<typename T>
    std::shared_ptr<T> getContext() noexcept
    {
        return std::static_pointer_cast<T>(context_);
    }

    void setTimeoutOff(
        size_t timeout,
        const std::shared_ptr<TimerWheel>& timerWheel) noexcept;

    void shutdown() noexcept;
    void forceClose() noexcept;

    void send(const char* buf, size_t len) noexcept;
    void send(std::string str) noexcept;
    void send(const TcpBuffer& buf) noexcept;

private:
    enum Status
    {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected,
    };

    class OffEntry
    {
    public:
        explicit OffEntry(std::weak_ptr<TcpConnection>&& connWeak) noexcept
        : connWeak_(std::move(connWeak)) { }

        ~OffEntry() noexcept
        {
            auto conn = connWeak_.lock();
            if (conn)
            {
                conn->forceClose();
            }
        }

    private:
        const std::weak_ptr<TcpConnection> connWeak_;
    };

    void sendInLoop(const char* buf, size_t len) noexcept;
    void extendLife() noexcept;

    void handleRead() noexcept;
    void handleWrite() noexcept;
    void handleClose() noexcept;
    void handleError() noexcept;

private:
    EventLoop* loop_;
    Socket socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    Status status_{kDisconnected};

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;

    TcpBuffer inputBuf_;
    TcpBuffer outputBuf_;

    std::shared_ptr<void> context_;

    std::weak_ptr<OffEntry> offEntry_;
    std::weak_ptr<TimerWheel> timerWheel_;
    size_t timeout_{0};
    Date lastUpdateTime_;
};

} // namespace god

#endif