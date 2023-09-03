#ifndef GOD_NET_POLLER_H
#define GOD_NET_POLLER_H

#include <sys/epoll.h>

#include <vector>
#include <unordered_set>

#include "god/utils/NonCopyable.h"

namespace god
{

class EventLoop;
class Channel;

/// 事件轮询
class Poller : NonCopyable
{
public:
    explicit Poller(EventLoop* loop) noexcept;
    ~Poller() noexcept;

    void wait(std::vector<Channel*>& readyChannels) noexcept;
    void change(Channel* channel) noexcept;

private:
    void ctl(int op, Channel* channel) noexcept;

    EventLoop* loop_;
    const int epfd_;

    std::vector<struct epoll_event> events_;
    std::unordered_set<Channel*> channelSet_;
};

} // namespace god

#endif