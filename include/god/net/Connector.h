#ifndef GOD_NET_CONNECTOR_H
#define GOD_NET_CONNECTOR_H

#include <memory>
#include <functional>

#include "god/utils/NonCopyable.h"
#include "god/net/InetAddress.h"
#include "god/net/Timer.h"
#include "god/net/EventLoop.h"
#include "god/net/Channel.h"

namespace god
{

/// 发起Tcp连接
class Connector : NonCopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    static constexpr double kInitDelay = 0.5;
    static constexpr double kMaxDelay = 30;

    Connector(EventLoop* loop, const InetAddress& serverAddr) noexcept;
    ~Connector() noexcept;

    void setNewConnectionCallback(NewConnectionCallback&& cb) noexcept
    {
        newConnectionCallback_ = std::move(cb);
    }

    void connect() noexcept;
    void reconnect() noexcept;

private:
    void retry(int sockfd) noexcept;
    int resetChannel() noexcept;

    void handleWrite() noexcept;

    enum Status
    {
        kConnecting,
        kConnected,
        kDisconnected,
    };

    EventLoop* loop_;
    InetAddress serverAddr_;
    double delay_{kInitDelay};

    Status status_{kDisconnected};
    std::shared_ptr<Channel> channel_;
    TimerId timerId_{0};

    NewConnectionCallback newConnectionCallback_;
};

} // namespace god

#endif