#ifndef GOD_NET_TIMER_H
#define GOD_NET_TIMER_H

#include <memory>
#include <functional>

#include "god/utils/NonCopyable.h"
#include "god/utils/Date.h"

namespace god
{

class Timer;

using TimerId = uint64_t;
using TimerPtr = std::shared_ptr<Timer>;
using TimerCallback = std::function<void()>;

/// 定时器
class Timer : NonCopyable
{
public:
    Timer(TimerCallback&& cb, Date when, double interval) noexcept;

    void run() const noexcept
    {
        callback_();
    }

    bool isRepeat() const noexcept
    {
        return interval_ > 0.0;
    }

    Date when() const noexcept
    {
        return when_;
    }

    TimerId id() const noexcept
    {
        return id_;
    }

    void restart(Date now) noexcept
    {
        when_ = now + interval_;
    }

private:
    const TimerCallback callback_;
    Date when_;
    const double interval_;
    const TimerId id_;
};

inline bool operator>(const TimerPtr& lhs, const TimerPtr& rhs) noexcept
{
    return lhs->when() > rhs->when();
}

} // namespace god

#endif