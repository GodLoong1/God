#include "god/net/Connector.h"

#include <cassert>

#include "god/utils/Logger.h"
#include "god/net/Socket.h"

namespace god
{

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr) noexcept
: loop_(loop),
  serverAddr_(serverAddr)
{
    LOG_TRACE << getThreadName() << ": addr "
              << serverAddr_.toIpPort();
}

Connector::~Connector() noexcept
{
    loop_->assertInLoop();

    if (timerId_)
    {
        loop_->cancelTimer(timerId_);
        timerId_ = 0;
    }
    if (status_ == kConnecting)
    {
        int sockfd = resetChannel();
        Socket::Close(sockfd);
        status_ = kDisconnected;
    }

    assert(!timerId_);
    assert(status_ != kConnecting);
    assert(!channel_);

    LOG_TRACE << getThreadName() << ": addr "
              << serverAddr_.toIpPort();
}

void Connector::connect() noexcept
{
    loop_->assertInLoop();
    assert(status_ == kDisconnected);
    assert(!channel_);
    assert(newConnectionCallback_);

    if (timerId_)
    {
        timerId_ = 0;
    }

    int sockfd = Socket::Create(serverAddr_.family());
    if (!Socket::Connect(sockfd, serverAddr_))
    {
        status_ = kConnecting;
        channel_.reset(new Channel(loop_, sockfd));
        channel_->setWriteCallback([this] { handleWrite(); });
        // 防止 restChannel 中析构
        channel_->tie(channel_);
        channel_->enableWriting();
    }
    else
    {
        LOG_WARN << strerr();
        retry(sockfd);
    }
}

void Connector::reconnect() noexcept
{
    loop_->assertInLoop();
    assert(status_ == kConnected);
    assert(!channel_);

    status_ = kDisconnected;
    delay_ = kInitDelay;
    connect();
}

void Connector::retry(int sockfd) noexcept
{
    loop_->assertInLoop();
    assert(status_ == kDisconnected);
    assert(!channel_);
    assert(!timerId_);

    Socket::Close(sockfd);
    timerId_ = loop_->runOnce(delay_, [this] { connect(); });

    delay_ *= 2;
    if (delay_ > kMaxDelay)
        delay_ = kMaxDelay;
}

int Connector::resetChannel() noexcept
{
    loop_->assertInLoop();
    assert(status_ == kConnecting);
    assert(channel_);

    channel_->disableAll();
    int sockfd = channel_->fd();
    channel_.reset();

    return sockfd;
}

void Connector::handleWrite() noexcept
{
    loop_->assertInLoop();
    assert(status_ == kConnecting);

    int sockfd = resetChannel();
    int err = Socket::GetSocketError(sockfd);

    if (err || Socket::isSelfConnect(sockfd))
    {
        LOG_ERROR << (err ? strerr(err) : "isSelfConnect!!!");

        status_ = kDisconnected;
        retry(sockfd);
    }
    else
    {
        status_ = kConnected;
        newConnectionCallback_(sockfd);
    }
}

} // namespace god