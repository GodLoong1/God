#include "god/net/TcpConnection.h"

#include <cassert>

#include "god/utils/Logger.h"
#include "god/net/EventLoop.h"
#include "god/net/Channel.h"
#include "god/net/TimerWheel.h"

namespace god
{

TcpConnection::TcpConnection(EventLoop* loop,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr) noexcept
: loop_(loop),
  socket_(sockfd),
  channel_(new Channel(loop_, sockfd)),
  localAddr_(localAddr),
  peerAddr_(peerAddr)
{
    channel_->setReadCallback([this] { handleRead(); });
    channel_->setWriteCallback([this] { handleWrite(); });
    channel_->setCloseCallback([this] { handleClose(); });
    channel_->setErrorCallback([this] { handleError(); });
    socket_.setKeepAlive(true);
}

TcpConnection::~TcpConnection() noexcept
{
    assert(status_ == kDisconnected);
    LOG_TRACE << "fd " << fd();
}

void TcpConnection::init() noexcept
{
    loop_->runInLoop([self(shared_from_this())] {
        assert(self->status_ == kDisconnected);
        LOG_TRACE << "fd " << self->fd();

        self->channel_->tie(self->weak_from_this());
        self->channel_->enableReading();
        self->status_ = kConnected;

        self->connectionCallback_(self);
    });
}

void TcpConnection::setTimeoutOff(
    size_t timeout,
    const std::shared_ptr<TimerWheel>& timerWheel) noexcept
{
    auto entry = std::make_shared<OffEntry>(weak_from_this());
    timerWheel->insertEntry(timeout, entry);

    offEntry_ = entry;
    timerWheel_ = timerWheel;
    timeout_ = timeout;
}

void TcpConnection::shutdown() noexcept
{
    loop_->runInLoop([self{shared_from_this()}] {
        if (self->status_ == kConnected)
        {
            self->status_ = kDisconnecting;
            if (!self->channel_->isWriting())
            {
                self->socket_.shutdown();
            }
        }
    });
}

void TcpConnection::forceClose() noexcept
{
    loop_->runInLoop([self{shared_from_this()}] {
        if (self->status_ == kConnected || self->status_ == kDisconnecting)
        {
            self->status_ = kDisconnecting;
            self->handleClose();
        }
    });
}

void TcpConnection::send(const char* buf, size_t len) noexcept
{
    if (loop_->isInLoop())
    {
        if (status_ == kConnected)
        {
            sendInLoop(buf, len);
        }
    }
    else
    {
        loop_->addInLoop(
            [self(shared_from_this()), str(std::string(buf, len))] {
                if (self->status_ == kConnected)
                {
                    self->sendInLoop(str.data(), str.size());
                }
        });
    }
}

void TcpConnection::send(std::string str) noexcept
{
    if (loop_->isInLoop())
    {
        if (status_ == kConnected)
        {
            sendInLoop(str.data(), str.size());
        }
    }
    else
    {
        loop_->addInLoop([self(shared_from_this()), str(std::move(str))] {
            if (self->status_ == kConnected)
            {
                self->sendInLoop(str.data(), str.size());
            }
        });
    }
}

void TcpConnection::send(const TcpBuffer& buf) noexcept
{
    if (loop_->isInLoop())
    {
        if (status_ == kConnected)
        {
            sendInLoop(buf.readPeek(), buf.readByte());
        }
    }
    else
    {
        std::string str(buf.readPeek(), buf.readByte());
        loop_->addInLoop([self(shared_from_this()), str(std::move(str))] {
            if (self->status_ == kConnected)
            {
                self->sendInLoop(str.data(), str.size());
            }
        });
    }
}

void TcpConnection::sendInLoop(const char* buf, size_t len) noexcept
{
    loop_->assertInLoop();
    assert(status_ == kConnected);
    extendLife();

    ssize_t n = 0;
    if (!channel_->isWriting() && outputBuf_.empty())
    {
        n = socket_.write(buf, len);
        if (n < 0)
        {
            n = 0;
            LOG_ERROR << "fd " << fd() << " " << strerr();
        }
    }

    if (static_cast<size_t>(n) < len)
    {
        outputBuf_.write(buf + n, len - n);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::extendLife() noexcept
{
    loop_->assertInLoop();

    if (timeout_ > 0)
    {
        auto now = Date::SystemTime();
        if (now < lastUpdateTime_ + 1.0)
        {
            return;
        }
        lastUpdateTime_ = now;

        if (auto entry = offEntry_.lock())
        {
            if (auto wheel = timerWheel_.lock())
            {
                wheel->insertEntry(timeout_, entry);
            }
        }
    }
}

void TcpConnection::handleRead() noexcept
{
    loop_->assertInLoop();
    extendLife();

    ssize_t n = inputBuf_.readFd(socket_.fd());
    if (n > 0)
    {
        messageCallback_(shared_from_this(), inputBuf_);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        LOG_ERROR << "fd " << fd() << " " << strerr();
        handleClose();
    }
}

void TcpConnection::handleWrite() noexcept
{
    loop_->assertInLoop();
    extendLife();

    if (channel_->isWriting())
    {

        ssize_t n = socket_.write(outputBuf_.readPeek(),
                                  outputBuf_.readByte());
        if (n > 0)
        {
            outputBuf_.retrieve(n);
            if (outputBuf_.empty())
            {
                channel_->disableWriting();
                if (status_ == kDisconnecting)
                {
                    socket_.shutdown();
                }
            }
        }
        else
        {
            LOG_ERROR << "fd " << fd() << " " << strerr();
        }
    }
}

void TcpConnection::handleClose() noexcept
{
    loop_->assertInLoop();

    assert(status_ == kConnected || status_ == kDisconnecting);

    status_ = kDisconnected;
    channel_->disableAll();

    TcpConnectionPtr self(shared_from_this());
    connectionCallback_(self);
    closeCallback_(self);
}

void TcpConnection::handleError() noexcept
{
    int err = Socket::GetSocketError(socket_.fd());
    LOG_ERROR << "fd " << fd() << " " << strerr(err);
}

} // namespace god