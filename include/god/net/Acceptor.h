#ifndef GOD_NET_ACCEPTOR_H
#define GOD_NET_ACCEPTOR_H

#include <functional>
#include <memory>

#include "god/utils/NonCopyable.h"
#include "god/net/Socket.h"

namespace god
{

class EventLoop;
class InetAddress;
class Channel;

/// 接受Tcp连接
class Acceptor : NonCopyable
{
public:
    using NewConnectionCallback =
        std::function<void(int connfd, const InetAddress& peerAddr)>;

    Acceptor(EventLoop* loop,
             const InetAddress& listenAddr,
             bool reuseAddr,
             bool reusePort) noexcept;
    ~Acceptor() noexcept;

    void setNewConnectionCallback(NewConnectionCallback&& cb) noexcept
    {
        newConnectionCallback_ = std::move(cb);
    }

    void listen() noexcept;

private:
    void handleRead() noexcept;

    EventLoop* loop_;
    int idlefd_;
    Socket socket_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
};

} // namespace god

#endif