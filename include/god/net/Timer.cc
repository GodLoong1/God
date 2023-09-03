#include "god/net/Timer.h"

#include <atomic>

namespace god
{

static std::atomic<TimerId> createTimerId{0};

Timer::Timer(TimerCallback&& cb, Date when, double interval) noexcept
: callback_(std::move(cb)),
  when_(when),
  interval_(interval),
  id_(++createTimerId)
{
}

} // namespace god