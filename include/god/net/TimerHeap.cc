#include "god/net/TimerHeap.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cassert>

#include "god/utils/Logger.h"
#include "god/net/Timer.h"
#include "god/net/EventLoop.h"
#include "god/net/Channel.h"

namespace god
{

TimerHeap::TimerHeap(EventLoop* loop) noexcept
: loop_(loop),
  timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
  channel_(new Channel(loop_, timerfd_))
{
    LOG_TRACE << getThreadName() << ": TimerHeap fd: " << timerfd_;

    channel_->setReadCallback([this] { handleExpired(); });
    channel_->enableReading();
}

TimerHeap::~TimerHeap() noexcept
{
    loop_->assertInLoop();

    channel_->disableAll();
    ::close(timerfd_);

    LOG_TRACE << getThreadName() << ": ~TimerHeap fd: " << timerfd_;
}

TimerId TimerHeap::addTimer(TimerCallback&& cb, Date when,
                            double interval) noexcept
{
    TimerPtr timerPtr(new Timer(std::move(cb), when, interval));

    if (loop_->isInLoop())
    {
        addTimerInLoop(timerPtr);
    }
    else
    {
        loop_->addInLoop([this, timerPtr] {
            addTimerInLoop(timerPtr);
        });
    }
    return timerPtr->id();
}

void TimerHeap::cancelTimer(TimerId timerId) noexcept
{
    loop_->runInLoop([this, timerId] {
        [[maybe_unused]] size_t n = timerIdSet_.erase(timerId);
        assert(n == 1);
    });
}

void TimerHeap::addTimerInLoop(const TimerPtr& timerPtr) noexcept
{
    loop_->assertInLoop();

    [[maybe_unused]] auto it = timerIdSet_.insert(timerPtr->id());
    assert(it.second);

    if (insert(timerPtr))
    {
        resetTimer(timerPtr->when());
    }
}

bool TimerHeap::insert(const TimerPtr& timerPtr) noexcept
{
    loop_->assertInLoop();

    bool changeTime = false;
    if (timers_.empty() || timerPtr->when() < timers_.top()->when())
    {
        changeTime = true;
    }
    timers_.emplace(timerPtr);

    return changeTime;
}

TimerHeap::TimerPtrVec TimerHeap::getExpired(Date now) noexcept
{
    loop_->assertInLoop();

    TimerPtrVec expired;
    while (!timers_.empty())
    {
        const TimerPtr& timerPtr = timers_.top();

        // 已取消的定时器
        if (!timerIdSet_.count(timerPtr->id()))
        {
            timers_.pop();
        }
        // 已到期的定时器
        else if (timerPtr->when() < now)
        {
            expired.push_back(timerPtr);
            timers_.pop();
        }
        // 没到期退出
        else
        {
            break;
        }
    }
    assert(expired.size() > 0);
    return expired;
}

void TimerHeap::setRepeat(const TimerPtrVec& expired, Date now) noexcept
{
    loop_->assertInLoop();

    for (const TimerPtr& timerPtr : expired)
    {
        if (timerPtr->isRepeat())
        {
            timerPtr->restart(now);
            insert(timerPtr);
        }
        else
        {
            [[maybe_unused]] size_t n = timerIdSet_.erase(timerPtr->id());
            assert(n == 1);
        }
    }

    if (!timers_.empty())
    {
        resetTimer(timers_.top()->when());
    }
}

void TimerHeap::resetTimer(Date expiration) noexcept
{
    loop_->assertInLoop();

    Date diff = expiration - Date::SteadyTime();
    if (diff.count() < 1000)
    {
        diff = Date(1000);
    }

    struct itimerspec spec;
    ::memset(&spec, 0, sizeof(spec));

    spec.it_value.tv_nsec = diff.toNano();
    spec.it_value.tv_sec = diff.toSeconds();

    if(::timerfd_settime(timerfd_, 0, &spec, nullptr) < 0)
    {
        LOG_FATAL << "resetTimer timerfd_settime: " << strerr();
    }
}

void TimerHeap::handleExpired() noexcept
{
    loop_->assertInLoop();

    uint64_t val;
    if (::read(timerfd_, &val, sizeof(val)) != sizeof(val))
    {
        LOG_FATAL << "handleExpired read: " << strerr();
    }

    Date now(Date::SteadyTime());
    TimerPtrVec expired(getExpired(now));

    for (const TimerPtr& timerPtr : expired)
    {
        if (timerIdSet_.count(timerPtr->id()))
        {
            timerPtr->run();
        }
    }
    setRepeat(expired, now);
}

} // namespace god