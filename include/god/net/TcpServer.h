#ifndef GOD_NET_TCPSERVER_H
#define GOD_NET_TCPSERVER_H

#include <set>
#include <map>
#include <string_view>

#include "god/net/TcpConnection.h"
#include "god/net/EventLoop.h"
#include "god/net/EventLoopThread.h"
#include "god/net/EventLoopThreadPool.h"
#include "god/net/Acceptor.h"
#include "god/net/TimerWheel.h"

namespace god
{

/// Tcp服务器
class TcpServer : NonCopyable
{
public:
    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string_view& name = "TcpServer",
              bool reuseAddr = true,
              bool reusePort = true) noexcept;
    ~TcpServer() noexcept;

    void start() noexcept;
    void stop() noexcept;

    EventLoop* getLoop() noexcept
    {
        return loop_;
    } 

    const std::string& getName() const noexcept
    {
        return name_;
    }

    void setTimeoutOff(size_t seconds) noexcept
    {
        timeout_ = seconds;
    }

    void setConnectionCallback(ConnectionCallback&& cb) noexcept
    {
        connectionCallback_ = std::move(cb);
    }

    void setMessageCallback(MessageCallback&& cb) noexcept
    {
        messageCallback_ = std::move(cb);
    }

    const std::set<TcpConnectionPtr>& getConnections() const noexcept
    {
        return connSet_;
    }

    void setIoLoopNum(size_t num) noexcept
    {
        loopPool_.reset(new EventLoopThreadPool(num, "IoLoop"));
        loopPool_->start();
        ioLoop_ = loopPool_->getLoops();
    }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr) noexcept;
    void removeConnection(const TcpConnectionPtr& conn) noexcept;

private:
    EventLoop* loop_;
    std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> loopPool_;

    std::vector<EventLoop*> ioLoop_;
    size_t index_{0};

    std::map<EventLoop*, std::shared_ptr<TimerWheel>> timerWheelMap_;
    size_t timeout_{0};

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;

    std::set<TcpConnectionPtr> connSet_;
};

} // namespace god

#endif