#ifndef GOD_NET_CHANNEL_H
#define GOD_NET_CHANNEL_H

#include <sys/epoll.h>

#include <functional>
#include <memory>
#include <string>

#include "god/utils/NonCopyable.h"

namespace god
{

class EventLoop;

/// 事件渠道
class Channel : NonCopyable
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    EventLoop* getLoop() noexcept
    {
        return loop_;
    }

    int fd() const noexcept
    {
        return fd_;
    }

    // 延长生命周期，防止在事件处理时析构
    void tie(std::weak_ptr<void>&& obj) noexcept
    {
        tie_ = std::move(obj);
        tied_ = true;
    }

    uint32_t events() const noexcept
    {
        return events_;
    }

    uint32_t revents() const noexcept
    {
        return revents_;
    }

    void setRevents(uint32_t revents) noexcept
    {
        revents_ = revents;
    }

    void setReadCallback(EventCallback&& cb) noexcept
    {
        readCallback_ = std::move(cb);
    }

    void setWriteCallback(EventCallback&& cb) noexcept
    {
        writeCallback_ = std::move(cb);
    }

    void setCloseCallback(EventCallback&& cb) noexcept
    {
        closeCallback_ = std::move(cb);
    }

    void setErrorCallback(EventCallback&& cb) noexcept
    {
        errorCallback_ = std::move(cb);
    }

    void setEventCallback(EventCallback&& cb) noexcept
    {
        eventCallback_ = std::move(cb);
    }

    bool isNoneEvent() const noexcept
    {
        return events_ == 0;
    }

    bool isReading() const noexcept
    {
        return events_ & EPOLLIN;
    }

    bool isWriting() const noexcept
    {
        return events_ & EPOLLOUT;
    }

    void enableReading() noexcept
    {
        events_ |= EPOLLIN;
        change();
    }

    void enableWriting() noexcept
    {
        events_ |= EPOLLOUT;
        change();
    }

    void disableReading() noexcept
    {
        events_ &= ~EPOLLIN;
        change();
    }

    void disableWriting() noexcept
    {
        events_ &= ~EPOLLOUT;
        change();
    }

    void disableAll() noexcept
    {
        events_ = 0;
        change();
    }

    std::string eventString() const noexcept
    {
        return toString(events_);
    }

    std::string reventString() const noexcept
    {
        return toString(revents_);
    }

    void handleEvent() noexcept;

private:
    void handleEventSafe() noexcept;
    void change() noexcept;
    std::string toString(uint32_t ev) const noexcept;

    EventLoop* loop_;
    int fd_;

    std::weak_ptr<void> tie_;
    bool tied_{false};

    bool handing_{false};
    uint32_t events_{0};
    uint32_t revents_{0};

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
    EventCallback eventCallback_;
};

} // namespace god

#endif