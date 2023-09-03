#include "god/net/Poller.h"

#include <unistd.h>

#include <cassert>
#include <cstring>

#include "god/utils/Logger.h"
#include "god/net/Channel.h"
#include "god/net/EventLoop.h"

namespace god
{

Poller::Poller(EventLoop* loop) noexcept
: loop_(loop),
  epfd_(::epoll_create1(EPOLL_CLOEXEC)),
  events_(32)
{
    LOG_TRACE << getThreadName() << ": fd: " << epfd_;
}

Poller::~Poller() noexcept
{
    loop_->assertInLoop();

    assert(channelSet_.empty());
    ::close(epfd_);

    LOG_TRACE << getThreadName() << ": fd: " << epfd_;
}

void Poller::wait(std::vector<Channel*>& readyChannels) noexcept
{
    loop_->assertInLoop();

    int ready = ::epoll_wait(epfd_, events_.data(), events_.size(), -1);
    if (ready > 0)
    {
        if (static_cast<size_t>(ready) == events_.size())
        {
            events_.resize(ready * 2);
        }

        for (int i = 0; i != ready; ++i)
        {
            Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
            assert(channelSet_.count(channel));

            channel->setRevents(events_[i].events);
            readyChannels.emplace_back(channel);
        }
    }
    else
    {
        LOG_ERROR << strerr();
    }
}

void Poller::change(Channel* channel) noexcept
{
    loop_->assertInLoop();

    if (channelSet_.count(channel))
    {
        if (channel->isNoneEvent())
        {
            ctl(EPOLL_CTL_DEL, channel);

            [[maybe_unused]] auto n = channelSet_.erase(channel);
            assert(n == 1);
        }
        else
        {
            ctl(EPOLL_CTL_MOD, channel);
        }
    }
    else
    {
        assert(!channel->isNoneEvent());

        ctl(EPOLL_CTL_ADD, channel);

        [[maybe_unused]] auto it = channelSet_.insert(channel);
        assert(it.second);
    }
}

void Poller::ctl(int op, Channel* channel) noexcept
{
    loop_->assertInLoop();

    struct epoll_event ev;
    ::memset(&ev, 0, sizeof(ev));

    ev.data.ptr = channel;
    ev.events = channel->events();

    if (::epoll_ctl(epfd_, op, channel->fd(), &ev) < 0)
    {
        LOG_FATAL << strerr();
    }
}

} // namespace god