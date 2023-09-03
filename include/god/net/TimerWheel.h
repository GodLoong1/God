#ifndef GOD_NET_TIMERWHEEL_H
#define GOD_NET_TIMERWHEEL_H

#include <memory>
#include <functional>
#include <unordered_set>
#include <deque>
#include <vector>

#include "god/utils/NonCopyable.h"
#include "god/net/Timer.h"

namespace god
{

class EventLoop;

/// 时间轮
class TimerWheel : NonCopyable 
{
public:
    using EntryPtr = std::shared_ptr<void>;
    using EntryBucket = std::unordered_set<EntryPtr>;
    using BucketQueue = std::deque<EntryBucket>;

    /**
     * @param maxTimeout 最大定时时间
     * @param ticksInterval 刻度间隔
     * @param buckets 桶数
     */
    TimerWheel(EventLoop* loop,
               size_t maxTimeout,
               double ticksInterval = 1.0,
               size_t bucketNum = 100) noexcept;
    ~TimerWheel() noexcept;

    void insertEntry(size_t delay, EntryPtr&& entryPtr) noexcept;

    EventLoop* getLoop() const noexcept
    {
        return loop_;
    }

private:
    void insertEntryInLoop(size_t delay, EntryPtr&& entryPtr) noexcept;
    void onTimer() noexcept;

    class DelayEntry : NonCopyable
    {
    public:
        DelayEntry(std::function<void()>&& cb) noexcept
        : cb_(std::move(cb)) { }

        ~DelayEntry() noexcept 
        {
            cb_();
        }

    private:
        const std::function<void()> cb_;
    };

    EventLoop* loop_;
    double ticksInterval_; // 刻度间隔
    size_t bucketNum_; // 桶数

    std::vector<BucketQueue> wheels_; // 时间轮
    size_t tickCounter_{0}; // 刻度计数
    TimerId timerId_{0}; // 定时器id
};

} // namespace god

#endif