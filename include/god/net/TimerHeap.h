#ifndef GOD_NET_TIMERHEAP_H
#define GOD_NET_TIMERHEAP_H

#include <memory>
#include <queue>
#include <unordered_set>

#include "god/utils/NonCopyable.h"
#include "god/utils/Date.h"
#include "god/net/Timer.h"

namespace god
{

class EventLoop;
class Channel;

/// 时间堆
class TimerHeap : NonCopyable
{
public:
    using TimerPtrVec = std::vector<TimerPtr>;

    explicit TimerHeap(EventLoop* loop) noexcept;
    ~TimerHeap() noexcept;

    TimerId addTimer(TimerCallback&& cb, Date when, double interval) noexcept;
    void cancelTimer(TimerId timerId) noexcept;

private:
    void addTimerInLoop(const TimerPtr& timerPtr) noexcept;

    // 插入定时器到堆中
    bool insert(const TimerPtr& timerPtr) noexcept;
    // 获取已过期的定时器
    TimerPtrVec getExpired(Date now) noexcept;
    // 设置重复定时器
    void setRepeat(const TimerPtrVec& expired, Date now) noexcept;
    // 重置定时器时间
    void resetTimer(Date expiration) noexcept;
    // 处理已过期的定时器
    void handleExpired() noexcept;

private:
    EventLoop* const loop_;
    const int timerfd_;
    const std::unique_ptr<Channel> channel_;

    std::priority_queue<TimerPtr, TimerPtrVec, std::greater<TimerPtr>> timers_;
    std::unordered_set<TimerId> timerIdSet_;
};

} // namespace god

#endif