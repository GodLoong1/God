#include "god/net/TcpServer.h"

#include <csignal>
#include <future>
#include <tuple>

#include "god/utils/Logger.h"

namespace god
{

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const std::string_view& name,
                     bool reuseAddr,
                     bool reusePort) noexcept
: loop_(loop),
  name_(name),
  acceptor_(new Acceptor(loop_, listenAddr, reuseAddr, reusePort)),
  ioLoop_{loop_}
{
    LOG_TRACE << "TcpServer() " << name_;

    ::signal(SIGPIPE, SIG_IGN);
}

TcpServer::~TcpServer() noexcept
{
    LOG_TRACE << "~TcpServer() " << name_;
}

void TcpServer::start() noexcept
{
    loop_->runInLoop([this] {
        if (timeout_ > 0)
        {
            for (EventLoop* loop : ioLoop_)
            {
                timerWheelMap_[loop] =
                    std::make_shared<TimerWheel>(loop,
                        timeout_, 1.0, timeout_ < 500
                        ? timeout_ + 1 : 100);
            }
        }
        acceptor_->setNewConnectionCallback(
            [this](int fd, const InetAddress& addr) {
                newConnection(fd, addr);
        });
        acceptor_->listen();
    });
}

void TcpServer::stop() noexcept
{
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    loop_->runInLoop([this, &promise] {
        acceptor_.reset();
        {
            std::vector<TcpConnectionPtr>
                connPtrs(connSet_.begin(), connSet_.end());
            for (auto& conn : connPtrs)
            {
                conn->forceClose();
            }
        }

        for (auto& iter : timerWheelMap_)
        {
            std::promise<void> promise;
            std::future<void> future = promise.get_future();

            iter.second->getLoop()->runInLoop([&iter, &promise] {
                iter.second.reset();
                promise.set_value();
            });

            future.get();
        }

        if (loopPool_)
        {
            loopPool_.reset();
        }
        promise.set_value();
    });
    future.get();
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) noexcept
{
    loop_->assertInLoop();

    EventLoop* ioLoop = ioLoop_[index_];
    if (++index_ == ioLoop_.size())
    {
        index_ = 0;
    }

    TcpConnectionPtr conn(
        new TcpConnection(ioLoop, sockfd,
                          Socket::GetLocalAddr(sockfd), peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback([this](const TcpConnectionPtr& conn) {
        removeConnection(conn);
    });

    if (timeout_ > 0)
    {
        conn->setTimeoutOff(timeout_, timerWheelMap_[ioLoop]);
    }

    conn->init();
    connSet_.emplace(std::move(conn));

    LOG_INFO << "在线tcp连接数量: " << connSet_.size();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) noexcept
{
    if (loop_->isInLoop())
    {
        [[maybe_unused]] size_t n = connSet_.erase(conn);
        assert(n == 1);
    }
    else
    {
        loop_->addInLoop([this, conn] {
            [[maybe_unused]] size_t n = connSet_.erase(conn);
            assert(n == 1);
        });
    }
    LOG_INFO << "在线tcp连接数量: " << connSet_.size();
}

} // namespace god