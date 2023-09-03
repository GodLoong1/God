#include "god/net/TcpClient.h"

#include <csignal>
#include <future>

#include "god/utils/Logger.h"

namespace god
{

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr,
                     const std::string_view& name) noexcept
: loop_(loop),
  name_(name),
  connector_(new Connector(loop, serverAddr))
{
    LOG_TRACE << getThreadName() << ": " << name_;

    ::signal(SIGPIPE, SIG_IGN);
}

TcpClient::~TcpClient()
{
    LOG_TRACE << getThreadName() << ": " << name_;
}

void TcpClient::start() noexcept
{
    loop_->runInLoop([this] {
        started_ = true;

        connector_->setNewConnectionCallback([this](int sockfd) {
            newConnection(sockfd);
        });
        connector_->connect();
    });
}

void TcpClient::stop() noexcept
{
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    loop_->runInLoop([this, &promise] {
        started_ = false;

        connector_.reset();
        if (connection_)
        {
            connection_->forceClose();
        }

        promise.set_value();
    });
    future.get();
}

void TcpClient::newConnection(int sockfd) noexcept
{
    loop_->assertInLoop();

    connection_.reset(
        new TcpConnection(loop_, sockfd, Socket::GetLocalAddr(sockfd),
                          Socket::GetPeerAddr(sockfd)));
    
    connection_->setConnectionCallback(connectionCallback_);
    connection_->setMessageCallback(messageCallback_);
    connection_->setCloseCallback([this](const TcpConnectionPtr& conn) {
        removeConnection(conn);
    });
    connection_->init();
}

void TcpClient::removeConnection(const TcpConnectionPtr&) noexcept
{
    loop_->assertInLoop();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_.reset();
    }

    if (retry_ && started_)
    {
        connector_->reconnect();
    }
}

} // namespace god