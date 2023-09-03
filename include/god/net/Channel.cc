#include "god/net/Channel.h"

#include <cassert>

#include "god/net/EventLoop.h"

namespace god
{

Channel::Channel(EventLoop* loop, int fd)
: loop_(loop),
  fd_(fd)
{
    assert(loop_);
    assert(fd_ >= 0);
}

Channel::~Channel()
{
    assert(!handing_);
    assert(isNoneEvent());
}

void Channel::handleEvent() noexcept
{
    loop_->assertInLoop();

    if (tied_)
    {
        if (auto guard = tie_.lock())
        {
            handleEventSafe();
        }
    }
    else
    {
        handleEventSafe();
    }
}

void Channel::handleEventSafe() noexcept
{
    loop_->assertInLoop();

    handing_ = true;

    if (eventCallback_)
    {
        eventCallback_();
        handing_ = false;
        return;
    }
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & EPOLLERR)
    {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if (readCallback_) readCallback_();
    }
    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_) writeCallback_();
    }

    handing_ = false;
}

void Channel::change() noexcept
{
    loop_->change(this);
}

std::string Channel::toString(uint32_t ev) const noexcept
{
    std::string ret;

    if (ev & EPOLLIN) ret += "IN ";
    if (ev & EPOLLPRI) ret += "PRI ";
    if (ev & EPOLLOUT) ret += "OUT ";
    if (ev & EPOLLERR) ret += "ERR ";
    if (ev & EPOLLHUP) ret += "HUP ";
    if (ev & EPOLLRDHUP) ret += "RDHUP ";

    if (!ret.empty())
        ret.pop_back();
    else
        ret = "NONE";

    return ret;
}

} // namespace god