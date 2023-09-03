#ifndef GOD_NET_EVENTLOOP_H
#define GOD_NET_EVENTLOOP_H

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "god/utils/NonCopyable.h"
#include "god/utils/Date.h"
#include "god/net/Timer.h"

namespace god
{

class Channel;
class Poller;
class TimerHeap;

/// 事件循环
class EventLoop : NonCopyable
{
public:
    using Func = std::function<void()>;

    static constexpr size_t nindex{size_t(-1)};

    EventLoop() noexcept;
    ~EventLoop() noexcept;

    void loop() noexcept;
    void quit() noexcept;

    void change(Channel* channel) noexcept;

    void runInLoop(Func&& cb) noexcept;
    void addInLoop(Func&& cb) noexcept;

    TimerId runAt(Date when, TimerCallback&& cb) noexcept;
    TimerId runOnce(double delay, TimerCallback&& cb) noexcept;
    TimerId runEvery(double interval, TimerCallback&& cb) noexcept;
    void cancelTimer(TimerId timerId) noexcept;

    bool isInLoop() const noexcept;
    void assertInLoop() const noexcept;

    size_t getIndex() const noexcept
    {
        return index_;
    }

    void setIndex(size_t index) noexcept
    {
        index_ = index;
    }

    static EventLoop* GetLoop() noexcept;

private:
    void handleRead() noexcept;
    void handleWrite() noexcept;

    const int tid_;
    std::atomic<bool> looping_;

    std::unique_ptr<Poller> poller_;
    std::vector<Channel*> readyChannels_;

    int wakeFd_;
    std::atomic_flag wakeing_;
    std::unique_ptr<Channel> wakeChannel_;
    std::mutex wakeMutex_;
    std::vector<Func> wakeCallbacks_;
    std::vector<Func> wakeTemp_;

    std::unique_ptr<TimerHeap> timerHeap_;

    size_t index_{nindex};
};

} // namespace god

#endif