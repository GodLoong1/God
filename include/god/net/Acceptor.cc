#include "god/net/Acceptor.h"

#include <fcntl.h>
#include <unistd.h>

#include <cassert>

#include "god/utils/Logger.h"
#include "god/net/EventLoop.h"
#include "god/net/InetAddress.h"
#include "god/net/Channel.h"

namespace god
{

Acceptor::Acceptor(EventLoop* loop,
                   const InetAddress& listenAddr,
                   bool reuseAddr,
                   bool reusePort) noexcept
: loop_(loop),
  idlefd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
  socket_(Socket::Create(listenAddr.family())),
  channel_(new Channel(loop_, socket_.fd()))
{
    LOG_TRACE << getThreadName() << ": idle fd: " << idlefd_; 
    LOG_TRACE << getThreadName() << ": fd: " << socket_.fd();

    socket_.setReuseAddr(reuseAddr);
    socket_.setReusePort(reusePort);
    socket_.bind(listenAddr);

    channel_->setReadCallback([this] { handleRead(); });
}

Acceptor::~Acceptor() noexcept
{
    loop_->assertInLoop();

    LOG_TRACE << getThreadName() << ": idle fd: " << idlefd_; 
    LOG_TRACE << getThreadName() << ": fd: " << socket_.fd();

    channel_->disableAll();
    ::close(idlefd_);
}

void Acceptor::listen() noexcept
{
    loop_->assertInLoop();
    assert(newConnectionCallback_);

    socket_.listen();
    channel_->enableReading();
}

void Acceptor::handleRead() noexcept
{
    loop_->assertInLoop();

    InetAddress peerAddr;
    if (int connfd = socket_.accept(peerAddr); connfd >= 0)
    {
        newConnectionCallback_(connfd, peerAddr);
    }
    else
    {
        LOG_ERROR << strerr();
        if (errno == EMFILE)
        {
            ::close(idlefd_);
            idlefd_ = socket_.accept(peerAddr);
            ::close(idlefd_);
            idlefd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace god